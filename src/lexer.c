#include "lexer.h"

#include "snuk_string.h"
#include "logger.h"

#include <errno.h>
#include <stdlib.h>

typedef struct KeyWord {
    const char *str;
    uint64_t len;
    SnukTokenType type;
} KeyWord;

typedef struct Value {
    const char *str;
    uint64_t len;
    SnukTokenType type;
    bool ignore_case;
} Value;

KeyWord keywords[] = {
    {.str = "var",      .len = 3, .type = SNUK_TOKEN_VAR},
    {.str = "const",    .len = 5, .type = SNUK_TOKEN_CONST},
    {.str = "if",       .len = 2, .type = SNUK_TOKEN_IF},
    {.str = "else",     .len = 4, .type = SNUK_TOKEN_ELSE},
    {.str = "match",    .len = 5, .type = SNUK_TOKEN_MATCH},
    {.str = "case",     .len = 4, .type = SNUK_TOKEN_CASE},
    {.str = "while",    .len = 5, .type = SNUK_TOKEN_WHILE},
    {.str = "do",       .len = 2, .type = SNUK_TOKEN_DO},
    {.str = "for",      .len = 3, .type = SNUK_TOKEN_FOR},
    {.str = "return",   .len = 6, .type = SNUK_TOKEN_RETURN},
    {.str = "break",    .len = 5, .type = SNUK_TOKEN_BREAK},
    {.str = "continue", .len = 8, .type = SNUK_TOKEN_CONTINUE},
    {.str = "fn",       .len = 2, .type = SNUK_TOKEN_FN},
    {.str = "print",    .len = 5, .type = SNUK_TOKEN_PRINT},
    {.str = "self",     .len = 4, .type = SNUK_TOKEN_SELF},
    {.str = "type",     .len = 4, .type = SNUK_TOKEN_TYPE},
    {.str = "or",       .len = 2, .type = SNUK_TOKEN_LOGICAL_OR},
    {.str = "and",      .len = 3, .type = SNUK_TOKEN_LOGICAL_AND},
    {.str = "not",      .len = 3, .type = SNUK_TOKEN_BANG},
};

Value values[] = {
    {.str = "true",     .len = 4, .type = SNUK_TOKEN_TRUE,  .ignore_case = false},
    {.str = "false",    .len = 5, .type = SNUK_TOKEN_FALSE, .ignore_case = false},
    {.str = "null",     .len = 4, .type = SNUK_TOKEN_NULL,  .ignore_case = false},

    {.str = "nan",      .len = 3, .type = SNUK_TOKEN_NAN,   .ignore_case = true},
    {.str = "inf",      .len = 3, .type = SNUK_TOKEN_INF,   .ignore_case = true},
    {.str = "infinity", .len = 8, .type = SNUK_TOKEN_INF,   .ignore_case = true},
};

SNUK_INLINE bool lexer_is_eof(SnukLexer *lexer) {
    return *lexer->cur == '\0';
}

SNUK_INLINE char lexer_peek(SnukLexer *lexer) {
    return *lexer->cur;
}

SNUK_INLINE char lexer_peek_next(SnukLexer *lexer) {
    if (lexer_is_eof(lexer)) return '\0';
    return *(lexer->cur + 1);
}

SNUK_INLINE char lexer_advance(SnukLexer *lexer) {
    if (lexer_is_eof(lexer)) return '\0';

    char c = *lexer->cur;

    lexer->cur++;

    if (c == '\n') {
        lexer->col = 0;
        lexer->line++;
    } else {
        lexer->col++;
    }

    return c;
}

SNUK_INLINE bool lexer_match(SnukLexer *lexer, char expected) {
    if (lexer_peek(lexer) != expected) return false;
    lexer_advance(lexer);
    return true;
}

SNUK_INLINE void lexer_skip_white_spaces(SnukLexer *lexer) {
    while (char_in_string(lexer_peek(lexer), " \t\n")) lexer_advance(lexer);
}

SNUK_INLINE SnukToken lexer_build_token(SnukLexer *lexer, SnukTokenType type) {
    uint64_t len = lexer->cur - lexer->token_start;
    return (SnukToken){
        .type = type,
        .string_literal = {.value = lexer->token_start, .length = len},
        .line = lexer->token_start_line,
        .col = lexer->token_start_col,
        .err_msg = NULL,
    };
}

SNUK_INLINE SnukToken lexer_build_error_token(SnukLexer *lexer, const char *err_msg) {
    const char *line_start = lexer->cur - lexer->col;
    uint64_t len = 0;
    for (len = 0; line_start[len] && line_start[len] != '\n'; ++len);

    return (SnukToken) {
        .type = SNUK_TOKEN_ERROR,
        .string_literal = {.value = line_start, .length = len},
        .line = lexer->line,
        .col = lexer->col,
        .err_msg = err_msg,
    };
}

static SnukTokenType check_keyword(const char *s, uint64_t len);
static SnukTokenType check_values(const char *s, uint64_t len);
static SnukToken lexer_scan_word(SnukLexer *lexer);
static SnukToken lexer_scan_number(SnukLexer *lexer);
static SnukToken lexer_scan_string(SnukLexer *lexer, char quote);
static SnukToken lexer_scan_comment(SnukLexer *lexer, bool multi_line);

static SnukToken lexer_scan_word(SnukLexer *lexer) {
    // assumes the first character is valid for identifier
    lexer->token_start = lexer->cur;
    lexer->token_start_line = lexer->line;
    lexer->token_start_col = lexer->col;
    char c;
    while ((c = lexer_peek(lexer))) {
        if (!is_alpha_numeric(c) && c != '_') break;
        lexer_advance(lexer);
    }

    SnukTokenType type;
    type = check_keyword(lexer->token_start, lexer->cur - lexer->token_start);
    if (type != SNUK_TOKEN_EOF) 
        return lexer_build_token(lexer, type);

    type = check_values(lexer->token_start, lexer->cur - lexer->token_start);
    if (type != SNUK_TOKEN_EOF)
        return lexer_build_token(lexer, type);

    return lexer_build_token(lexer, SNUK_TOKEN_IDENTIFIER);
}

static SnukToken lexer_scan_number(SnukLexer *lexer) {
    lexer->token_start = lexer->cur;
    lexer->token_start_line = lexer->line;
    lexer->token_start_col = lexer->col;

    bool is_float = false;
    int base = 10;
    SnukToken token = {
        .line = lexer->line,
        .col = lexer->col,
        .err_msg = NULL,
    };

    // detect base
    if (lexer_peek(lexer) == '0') {
        switch (lexer_peek_next(lexer) | (1 << 5)) {
            case 'x':
                base = 16;
                lexer_advance(lexer);
                lexer_advance(lexer);
                if (!is_hex_digit(lexer_peek(lexer)))
                    return lexer_build_error_token(lexer, "invalid hex literal");
                while (is_hex_digit(lexer_peek(lexer))) lexer_advance(lexer);
                break;
            case 'b':
                base = 2;
                lexer_advance(lexer);
                lexer_advance(lexer);

                if (!is_binary_digit(lexer_peek(lexer)))
                    return lexer_build_error_token(lexer, "invalid binary literal");
                while (is_binary_digit(lexer_peek(lexer))) lexer_advance(lexer);
                break;
            default:
                if (is_octal_digit(lexer_peek(lexer))) {
                    base = 8;
                    lexer_advance(lexer);

                    while (is_octal_digit(lexer_peek(lexer))) lexer_advance(lexer);
                    break;
                }
                // single 0
                lexer_advance(lexer);
                break;
        }
    } else {
        while (is_digit(lexer_peek(lexer))) lexer_advance(lexer);
    }

    // fraction
    if (base == 10 && lexer_peek(lexer) == '.') {
        is_float = true;
        lexer_advance(lexer);

        while (is_digit(lexer_peek(lexer))) lexer_advance(lexer);
    }

    if (base == 10 && (lexer_peek(lexer) | (1 << 5) ) == 'e') {
        is_float = true;
        lexer_advance(lexer);

        if (char_in_string(lexer_peek(lexer), "+-")) lexer_advance(lexer);

        if (!is_digit(lexer_peek(lexer))) return lexer_build_error_token(lexer, "invalid exponent");

        while (is_digit(lexer_peek(lexer))) lexer_advance(lexer);
    }

    errno = 0;

    if (is_float) {
        char *endptr;

        token.float_literal = strtod(lexer->token_start, &endptr);

        if (errno == ERANGE)
            return lexer_build_error_token(lexer, "float literal out of range");
        token.type = SNUK_TOKEN_FLOAT;
    } else {
        if (base == 2) {
            token.int_literal = 0;
            const char *p = lexer->token_start + 2;
            while (char_in_string(*p, "01")) {
                if (token.int_literal > (INT64_MAX >> 1)) 
                    return lexer_build_error_token(lexer, "integer literal out of range");
                token.int_literal = (token.int_literal << 1) | (*p - '0');
                ++p;
            }
        } else {
            char *endptr;
            token.int_literal = strtoll(lexer->token_start, &endptr, base);
            if (errno == ERANGE)
                return lexer_build_error_token(lexer, "integer literal out of range");
        }

        token.type = SNUK_TOKEN_INTEGER;
    }

    return token;
}

static SnukToken lexer_scan_string(SnukLexer *lexer, char quote) {
    // starting quote is consumed

    while (lexer_peek(lexer) != quote && !lexer_is_eof(lexer))
        lexer_advance(lexer);

    if (lexer_is_eof(lexer)) return lexer_build_error_token(lexer, "unterminated string");

    lexer_advance(lexer); // consume closing quote

    return lexer_build_token(lexer, SNUK_TOKEN_STRING);
}

static SnukTokenType check_keyword(const char *s, uint64_t len) {
    for (uint64_t i = 0; i < ARRAY_LEN(keywords); ++i) {
        if (len != keywords[i].len) continue;

        if (string_n_equal(s, keywords[i].str, keywords[i].len))
            return keywords[i].type;
    }
    return SNUK_TOKEN_EOF;
}

static SnukTokenType check_values(const char *s, uint64_t len) {
    for (uint64_t i = 0; i < ARRAY_LEN(values); ++i) {
        if (len != values[i].len) continue;

        if (values[i].ignore_case && string_n_equal_ignore_case(s, values[i].str, values[i].len))
            return values[i].type;
        else if (string_n_equal(s, values[i].str, values[i].len))
            return values[i].type;
    }

    return SNUK_TOKEN_EOF;
}

static SnukToken lexer_scan_comment(SnukLexer *lexer, bool multi_line) {
    lexer->token_start = lexer->cur;
    lexer->token_start_line = lexer->line;
    lexer->token_start_col = lexer->col;

    if (!multi_line) {
        while (!lexer_is_eof(lexer) && lexer_peek(lexer) != '\n') lexer_advance(lexer);
        SnukToken token = lexer_build_token(lexer, SNUK_TOKEN_SLCOMMENT);
        lexer_advance(lexer); // consume newline
        return token;
    }

    while (!lexer_is_eof(lexer) && (lexer_peek(lexer) != '*' || lexer_peek_next(lexer) != '/'))
        lexer_advance(lexer);

    if (lexer_is_eof(lexer))
        return lexer_build_error_token(lexer, "unterminated multi-line comment");

    SnukToken token = lexer_build_token(lexer, SNUK_TOKEN_MLCOMMENT);
    lexer_advance(lexer); // consume *
    lexer_advance(lexer); // consume /
    return token;
}

SnukToken snuk_lexer_next_token(SnukLexer *lexer) {
    lexer_skip_white_spaces(lexer);
    lexer->token_start = lexer->cur;
    lexer->token_start_line = lexer->line;
    lexer->token_start_col = lexer->col;

    char c = lexer_peek(lexer);

    if (is_digit(c)) return lexer_scan_number(lexer);
    if (c == '.' && is_digit(lexer_peek_next(lexer)))
        return lexer_scan_number(lexer);

    if (is_alpha_numeric(c) || c == '_')
        return lexer_scan_word(lexer);

    switch (lexer_advance(lexer)) {
        case '\0':
            return lexer_build_token(lexer, SNUK_TOKEN_EOF);
        case '(':
            return lexer_build_token(lexer, SNUK_TOKEN_LPAREN);
        case ')':
            return lexer_build_token(lexer, SNUK_TOKEN_RPAREN);
        case '{':
            return lexer_build_token(lexer, SNUK_TOKEN_LBRACE);
        case '}':
            return lexer_build_token(lexer, SNUK_TOKEN_RBRACE);
        case '[':
            return lexer_build_token(lexer, SNUK_TOKEN_LBRACKET);
        case ']':
            return lexer_build_token(lexer, SNUK_TOKEN_RBRACKET);
        case '.':
            return lexer_build_token(lexer, SNUK_TOKEN_DOT);
        case ':':
            return lexer_build_token(lexer, SNUK_TOKEN_COLON);
        case ';':
            return lexer_build_token(lexer, SNUK_TOKEN_SEMICOLON);
        case '+':
            if (lexer_match(lexer, '+')) return lexer_build_token(lexer, SNUK_TOKEN_INCREMENT);
            if (lexer_match(lexer, '=')) return lexer_build_token(lexer, SNUK_TOKEN_PLUS_ASSIGN);
            return lexer_build_token(lexer, SNUK_TOKEN_PLUS);
        case '-':
            if (lexer_match(lexer, '>')) return lexer_build_token(lexer, SNUK_TOKEN_ARROW);
            if (lexer_match(lexer, '=')) return lexer_build_token(lexer, SNUK_TOKEN_MINUS_ASSIGN);
            return lexer_build_token(lexer, SNUK_TOKEN_MINUS);
        case '*':
            if (lexer_match(lexer, '='))
                return lexer_build_token(lexer, SNUK_TOKEN_STAR_ASSIGN);
            return lexer_build_token(lexer, SNUK_TOKEN_STAR);
        case '/':
            if (lexer_match(lexer, '=')) return lexer_build_token(lexer, SNUK_TOKEN_SLASH_ASSIGN);
            if (lexer_match(lexer, '/')) return lexer_scan_comment(lexer, false);
            if (lexer_match(lexer, '*')) return lexer_scan_comment(lexer, true);
            return lexer_build_token(lexer, SNUK_TOKEN_SLASH);
        case '%':
            if (lexer_match(lexer, '='))
                return lexer_build_token(lexer, SNUK_TOKEN_PERCENT_ASSIGN);
            return lexer_build_token(lexer, SNUK_TOKEN_PERCENT);
        case '<':
            if (lexer_match(lexer, '<')) {
                if (lexer_match(lexer, '=')) return lexer_build_token(lexer, SNUK_TOKEN_LEFT_SHIFT_ASSIGN);
                return lexer_build_token(lexer, SNUK_TOKEN_LEFT_SHIFT);
            }
            if (lexer_match(lexer, '=')) return lexer_build_token(lexer, SNUK_TOKEN_LESS_THAN_OR_EQUAL);
            return lexer_build_token(lexer, SNUK_TOKEN_LESS_THAN);
        case '>':
            if (lexer_match(lexer, '>')) {
                if (lexer_match(lexer, '=')) return lexer_build_token(lexer, SNUK_TOKEN_RIGHT_SHIFT_ASSIGN);
                return lexer_build_token(lexer, SNUK_TOKEN_RIGHT_SHIFT);
            }
            if (lexer_match(lexer, '=')) return lexer_build_token(lexer, SNUK_TOKEN_GREATER_THAN_OR_EQUAL);
            return lexer_build_token(lexer, SNUK_TOKEN_GREATER_THAN);
        case '=':
            if (lexer_match(lexer, '=')) return lexer_build_token(lexer, SNUK_TOKEN_EQUAL);
            return lexer_build_token(lexer, SNUK_TOKEN_ASSIGN);
        case '!':
            if (lexer_match(lexer, '=')) return lexer_build_token(lexer, SNUK_TOKEN_NOT_EQUAL);
            return lexer_build_token(lexer, SNUK_TOKEN_BANG);
        case '&':
            if (lexer_match(lexer, '&')) return lexer_build_token(lexer, SNUK_TOKEN_LOGICAL_AND);
            if (lexer_match(lexer, '=')) return lexer_build_token(lexer, SNUK_TOKEN_AND_ASSIGN);
            return lexer_build_token(lexer, SNUK_TOKEN_AND);
        case '|':
            if (lexer_match(lexer, '|')) return lexer_build_token(lexer, SNUK_TOKEN_LOGICAL_OR);
            if (lexer_match(lexer, '=')) return lexer_build_token(lexer, SNUK_TOKEN_OR_ASSIGN);
            return lexer_build_token(lexer, SNUK_TOKEN_OR);
        case '~':
            return lexer_build_token(lexer, SNUK_TOKEN_TILDE);
        case '^':
            if (lexer_match(lexer, '=')) return lexer_build_token(lexer, SNUK_TOKEN_XOR_ASSIGN);
            return lexer_build_token(lexer, SNUK_TOKEN_XOR);
        case ',':
            return lexer_build_token(lexer, SNUK_TOKEN_COMMA);

        case '"':
        case '\'':
            return lexer_scan_string(lexer, c);

        default:
            break;
    }

    return lexer_build_error_token(lexer, "unexpected character");
}

void snuk_lexer_log_token(SnukToken token) {
    log_trace("Token type: %s", snuk_lexer_token_type_to_string(token.type));
    if (token.type == SNUK_TOKEN_INTEGER)
        log_trace("\tInteger value: %ld", token.int_literal);
    else if (token.type == SNUK_TOKEN_FLOAT)
        log_trace("\tFloat value: %lf", token.float_literal);
    else if (token.type == SNUK_TOKEN_ERROR)
        log_trace("\tError line: %.*s and err_msg: %s",
                token.string_literal.length, token.string_literal.value, token.err_msg);
    else
        log_trace("\tString value: %.*s", token.string_literal.length, token.string_literal.value);
    log_trace("\tLine: %d, Column: %d", token.line, token.col);
}

const char *snuk_lexer_token_type_to_string(SnukTokenType type) {
    switch (type) {
        case SNUK_TOKEN_EOF:
            return SNUK_STRINGIFY(SNUK_TOKEN_EOF);
        case SNUK_TOKEN_ERROR:
            return SNUK_STRINGIFY(SNUK_TOKEN_ERROR);
        case SNUK_TOKEN_IDENTIFIER:
            return SNUK_STRINGIFY(SNUK_TOKEN_IDENTIFIER);
        case SNUK_TOKEN_INTEGER:
            return SNUK_STRINGIFY(SNUK_TOKEN_INTEGER);
        case SNUK_TOKEN_FLOAT:
            return SNUK_STRINGIFY(SNUK_TOKEN_FLOAT);
        case SNUK_TOKEN_STRING:
            return SNUK_STRINGIFY(SNUK_TOKEN_STRING);
        case SNUK_TOKEN_TRUE:
            return SNUK_STRINGIFY(SNUK_TOKEN_TRUE);
        case SNUK_TOKEN_FALSE:
            return SNUK_STRINGIFY(SNUK_TOKEN_FALSE);
        case SNUK_TOKEN_NULL:
            return SNUK_STRINGIFY(SNUK_TOKEN_NULL);
        case SNUK_TOKEN_NAN:
            return SNUK_STRINGIFY(SNUK_TOKEN_NAN);
        case SNUK_TOKEN_INF:
            return SNUK_STRINGIFY(SNUK_TOKEN_INF);
        case SNUK_TOKEN_VAR:
            return SNUK_STRINGIFY(SNUK_TOKEN_VAR);
        case SNUK_TOKEN_CONST:
            return SNUK_STRINGIFY(SNUK_TOKEN_CONST);
        case SNUK_TOKEN_IF:
            return SNUK_STRINGIFY(SNUK_TOKEN_IF);
        case SNUK_TOKEN_ELSE:
            return SNUK_STRINGIFY(SNUK_TOKEN_ELSE);
        case SNUK_TOKEN_MATCH:
            return SNUK_STRINGIFY(SNUK_TOKEN_MATCH);
        case SNUK_TOKEN_CASE:
            return SNUK_STRINGIFY(SNUK_TOKEN_CASE);
        case SNUK_TOKEN_WHILE:
            return SNUK_STRINGIFY(SNUK_TOKEN_WHILE);
        case SNUK_TOKEN_DO:
            return SNUK_STRINGIFY(SNUK_TOKEN_DO);
        case SNUK_TOKEN_FOR:
            return SNUK_STRINGIFY(SNUK_TOKEN_FOR);
        case SNUK_TOKEN_RETURN:
            return SNUK_STRINGIFY(SNUK_TOKEN_RETURN);
        case SNUK_TOKEN_BREAK:
            return SNUK_STRINGIFY(SNUK_TOKEN_BREAK);
        case SNUK_TOKEN_CONTINUE:
            return SNUK_STRINGIFY(SNUK_TOKEN_CONTINUE);
        case SNUK_TOKEN_FN:
            return SNUK_STRINGIFY(SNUK_TOKEN_FN);
        case SNUK_TOKEN_PRINT:
            return SNUK_STRINGIFY(SNUK_TOKEN_PRINT);
        case SNUK_TOKEN_SELF:
            return SNUK_STRINGIFY(SNUK_TOKEN_SELF);
        case SNUK_TOKEN_TYPE:
            return SNUK_STRINGIFY(SNUK_TOKEN_TYPE);
        case SNUK_TOKEN_LPAREN:
            return SNUK_STRINGIFY(SNUK_TOKEN_LPAREN);
        case SNUK_TOKEN_RPAREN:
            return SNUK_STRINGIFY(SNUK_TOKEN_RPAREN);
        case SNUK_TOKEN_LBRACE:
            return SNUK_STRINGIFY(SNUK_TOKEN_LBRACE);
        case SNUK_TOKEN_RBRACE:
            return SNUK_STRINGIFY(SNUK_TOKEN_RBRACE);
        case SNUK_TOKEN_LBRACKET:
            return SNUK_STRINGIFY(SNUK_TOKEN_LBRACKET);
        case SNUK_TOKEN_RBRACKET:
            return SNUK_STRINGIFY(SNUK_TOKEN_RBRACKET);
        case SNUK_TOKEN_COMMA:
            return SNUK_STRINGIFY(SNUK_TOKEN_COMMA);
        case SNUK_TOKEN_SEMICOLON:
            return SNUK_STRINGIFY(SNUK_TOKEN_SEMICOLON);
        case SNUK_TOKEN_COLON:
            return SNUK_STRINGIFY(SNUK_TOKEN_COLON);
        case SNUK_TOKEN_DOT:
            return SNUK_STRINGIFY(SNUK_TOKEN_DOT);
        case SNUK_TOKEN_ARROW:
            return SNUK_STRINGIFY(SNUK_TOKEN_ARROW);
        case SNUK_TOKEN_PLUS:
            return SNUK_STRINGIFY(SNUK_TOKEN_PLUS);
        case SNUK_TOKEN_MINUS:
            return SNUK_STRINGIFY(SNUK_TOKEN_MINUS);
        case SNUK_TOKEN_STAR:
            return SNUK_STRINGIFY(SNUK_TOKEN_STAR);
        case SNUK_TOKEN_SLASH:
            return SNUK_STRINGIFY(SNUK_TOKEN_SLASH);
        case SNUK_TOKEN_PERCENT:
            return SNUK_STRINGIFY(SNUK_TOKEN_PERCENT);
        case SNUK_TOKEN_LEFT_SHIFT:
            return SNUK_STRINGIFY(SNUK_TOKEN_LEFT_SHIFT);
        case SNUK_TOKEN_RIGHT_SHIFT:
            return SNUK_STRINGIFY(SNUK_TOKEN_RIGHT_SHIFT);
        case SNUK_TOKEN_OR:
            return SNUK_STRINGIFY(SNUK_TOKEN_OR);
        case SNUK_TOKEN_AND:
            return SNUK_STRINGIFY(SNUK_TOKEN_AND);
        case SNUK_TOKEN_TILDE:
            return SNUK_STRINGIFY(SNUK_TOKEN_TILDE);
        case SNUK_TOKEN_XOR:
            return SNUK_STRINGIFY(SNUK_TOKEN_XOR);
        case SNUK_TOKEN_LOGICAL_AND:
            return SNUK_STRINGIFY(SNUK_TOKEN_LOGICAL_AND);
        case SNUK_TOKEN_LOGICAL_OR:
            return SNUK_STRINGIFY(SNUK_TOKEN_LOGICAL_OR);
        case SNUK_TOKEN_BANG:
            return SNUK_STRINGIFY(SNUK_TOKEN_BANG);
        case SNUK_TOKEN_ASSIGN:
            return SNUK_STRINGIFY(SNUK_TOKEN_ASSIGN);
        case SNUK_TOKEN_PLUS_ASSIGN:
            return SNUK_STRINGIFY(SNUK_TOKEN_PLUS_ASSIGN);
        case SNUK_TOKEN_MINUS_ASSIGN:
            return SNUK_STRINGIFY(SNUK_TOKEN_MINUS_ASSIGN);
        case SNUK_TOKEN_STAR_ASSIGN:
            return SNUK_STRINGIFY(SNUK_TOKEN_STAR_ASSIGN);
        case SNUK_TOKEN_SLASH_ASSIGN:
            return SNUK_STRINGIFY(SNUK_TOKEN_SLASH_ASSIGN);
        case SNUK_TOKEN_PERCENT_ASSIGN:
            return SNUK_STRINGIFY(SNUK_TOKEN_PERCENT_ASSIGN);
        case SNUK_TOKEN_LEFT_SHIFT_ASSIGN:
            return SNUK_STRINGIFY(SNUK_TOKEN_LEFT_SHIFT_ASSIGN);
        case SNUK_TOKEN_RIGHT_SHIFT_ASSIGN:
            return SNUK_STRINGIFY(SNUK_TOKEN_RIGHT_SHIFT_ASSIGN);
        case SNUK_TOKEN_OR_ASSIGN:
            return SNUK_STRINGIFY(SNUK_TOKEN_OR_ASSIGN);
        case SNUK_TOKEN_AND_ASSIGN:
            return SNUK_STRINGIFY(SNUK_TOKEN_AND_ASSIGN);
        case SNUK_TOKEN_XOR_ASSIGN:
            return SNUK_STRINGIFY(SNUK_TOKEN_XOR_ASSIGN);
        case SNUK_TOKEN_EQUAL:
            return SNUK_STRINGIFY(SNUK_TOKEN_EQUAL);
        case SNUK_TOKEN_NOT_EQUAL:
            return SNUK_STRINGIFY(SNUK_TOKEN_NOT_EQUAL);
        case SNUK_TOKEN_LESS_THAN:
            return SNUK_STRINGIFY(SNUK_TOKEN_LESS_THAN);
        case SNUK_TOKEN_GREATER_THAN:
            return SNUK_STRINGIFY(SNUK_TOKEN_GREATER_THAN);
        case SNUK_TOKEN_LESS_THAN_OR_EQUAL:
            return SNUK_STRINGIFY(SNUK_TOKEN_LESS_THAN_OR_EQUAL);
        case SNUK_TOKEN_GREATER_THAN_OR_EQUAL:
            return SNUK_STRINGIFY(SNUK_TOKEN_GREATER_THAN_OR_EQUAL);
        case SNUK_TOKEN_SLCOMMENT:
            return SNUK_STRINGIFY(SNUK_TOKEN_SLCOMMENT);
        case SNUK_TOKEN_MLCOMMENT:
            return SNUK_STRINGIFY(SNUK_TOKEN_MLCOMMENT);
        case SNUK_TOKEN_INCREMENT:
            return SNUK_STRINGIFY(SNUK_TOKEN_INCREMENT);
        case SNUK_TOKEN_DECREMENT:
            return SNUK_STRINGIFY(SNUK_TOKEN_DECREMENT);
        case SNUK_TOKEN_MAX:
            return SNUK_STRINGIFY(SNUK_TOKEN_MAX);
        default:
            return "Unkown token type";
    };
}
