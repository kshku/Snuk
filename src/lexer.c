#include "lexer.h"

#include "string.h"
#include "logger.h"

#include <errno.h>
#include <stdlib.h>

typedef struct KeyWord {
    const char *str;
    uint32_t len;
    TokenType type;
} KeyWord;

typedef struct Value {
    const char *str;
    uint32_t len;
    TokenType type;
    bool ignore_case;
} Value;

KeyWord keywords[] = {
    {.str = "var",      .len = 3, .type = TOKEN_TYPE_VAR},
    {.str = "const",    .len = 5, .type = TOKEN_TYPE_CONST},
    {.str = "if",       .len = 2, .type = TOKEN_TYPE_IF},
    {.str = "else",     .len = 4, .type = TOKEN_TYPE_ELSE},
    {.str = "match",    .len = 5, .type = TOKEN_TYPE_MATCH},
    {.str = "case",     .len = 4, .type = TOKEN_TYPE_CASE},
    {.str = "while",    .len = 5, .type = TOKEN_TYPE_WHILE},
    {.str = "do",       .len = 2, .type = TOKEN_TYPE_DO},
    {.str = "for",      .len = 3, .type = TOKEN_TYPE_FOR},
    {.str = "return",   .len = 6, .type = TOKEN_TYPE_RETURN},
    {.str = "break",    .len = 5, .type = TOKEN_TYPE_BREAK},
    {.str = "continue", .len = 8, .type = TOKEN_TYPE_CONTINUE},
    {.str = "fn",       .len = 2, .type = TOKEN_TYPE_FN},
    {.str = "print",    .len = 5, .type = TOKEN_TYPE_PRINT},
    {.str = "self",     .len = 4, .type = TOKEN_TYPE_SELF},
    {.str = "type",     .len = 4, .type = TOKEN_TYPE_TYPE},
    {.str = "or",       .len = 2, .type = TOKEN_TYPE_LOGICAL_OR},
    {.str = "and",      .len = 3, .type = TOKEN_TYPE_LOGICAL_AND},
    {.str = "not",      .len = 3, .type = TOKEN_TYPE_BANG},
};

Value values[] = {
    {.str = "true",     .len = 4, .type = TOKEN_TYPE_TRUE,  .ignore_case = false},
    {.str = "false",    .len = 5, .type = TOKEN_TYPE_FALSE, .ignore_case = false},
    {.str = "null",     .len = 4, .type = TOKEN_TYPE_NULL,  .ignore_case = false},

    {.str = "nan",      .len = 3, .type = TOKEN_TYPE_NAN,   .ignore_case = true},
    {.str = "inf",      .len = 3, .type = TOKEN_TYPE_INF,   .ignore_case = true},
    {.str = "infinity", .len = 8, .type = TOKEN_TYPE_INF,   .ignore_case = true},
};

SNUK_INLINE char lexer_peek(Lexer *lexer) {
    return *lexer->cur;
}

SNUK_INLINE char lexer_peek_next(Lexer *lexer) {
    if (lexer_is_eof(lexer)) return '\0';
    return *(lexer->cur + 1);
}

SNUK_INLINE char lexer_advance(Lexer *lexer) {
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

SNUK_INLINE bool lexer_match(Lexer *lexer, char expected) {
    if (*lexer->cur != expected) return false;
    lexer_advance(lexer);
    return true;
}

SNUK_INLINE void lexer_skip_white_spaces(Lexer *lexer) {
    while (char_in_string(lexer_peek(lexer), " \t\n")) lexer_advance(lexer);
}

SNUK_INLINE Token lexer_build_token(Lexer *lexer, TokenType type) {
    uint32_t len = lexer->cur - lexer->token_start;
    return (Token){
        .type = type,
        .string_value = {.string = lexer->token_start, .length = len},
        .line = lexer->token_start_line,
        .col = lexer->token_start_col,
        .err_msg = NULL,
    };
}

SNUK_INLINE Token lexer_build_error_token(Lexer *lexer, const char *err_msg) {
    const char *line_start = lexer->cur - lexer->col;
    uint32_t len = 0;
    for (len = 0; line_start[len] && line_start[len] != '\n'; ++len);

    return (Token) {
        .type = TOKEN_TYPE_ERROR,
        .string_value = {.string = line_start, .length = len},
        .line = lexer->line,
        .col = lexer->col,
        .err_msg = err_msg,
    };
}

static TokenType check_keyword(const char *s, uint32_t len);
static TokenType check_values(const char *s, uint32_t len);
static Token lexer_scan_word(Lexer *lexer);
static Token lexer_scan_number(Lexer *lexer);
static Token lexer_scan_string(Lexer *lexer, char quote);
static Token lexer_scan_comment(Lexer *lexer, bool multi_line);

static Token lexer_scan_word(Lexer *lexer) {
    // assumes the first character is valid for identifier
    lexer->token_start = lexer->cur;
    lexer->token_start_line = lexer->line;
    lexer->token_start_col = lexer->col;
    char c;
    while (c = lexer_peek(lexer)) {
        if (!is_alpha_numeric(c) && c != '_') break;
        lexer_advance(lexer);
    }

    TokenType type;
    type = check_keyword(lexer->token_start, lexer->cur - lexer->token_start);
    if (type != TOKEN_TYPE_EOF) 
        return lexer_build_token(lexer, type);

    type = check_values(lexer->token_start, lexer->cur - lexer->token_start);
    if (type != TOKEN_TYPE_EOF)
        return lexer_build_token(lexer, type);

    return lexer_build_token(lexer, TOKEN_TYPE_IDENTIFIER);
}

static Token lexer_scan_number(Lexer *lexer) {
    lexer->token_start = lexer->cur;
    lexer->token_start_line = lexer->line;
    lexer->token_start_col = lexer->col;

    bool is_float = false;
    int base = 10;
    Token token = {
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

        token.float_value = strtod(lexer->token_start, &endptr);

        if (errno == ERANGE)
            return lexer_build_error_token(lexer, "float literal out of range");
        token.type = TOKEN_TYPE_FLOAT;
    } else {
        if (base == 2) {
            token.int_value = 0;
            const char *p = lexer->token_start + 2;
            while (char_in_string(*p, "01")) {
                if (token.int_value > (INT64_MAX >> 1)) 
                    return lexer_build_error_token(lexer, "integer literal out of range");
                token.int_value = (token.int_value << 1) | (*p - '0');
                ++p;
            }
        } else {
            char *endptr;
            token.int_value = strtoll(lexer->token_start, &endptr, base);
            if (errno == ERANGE)
                return lexer_build_error_token(lexer, "integer literal out of range");
        }

        token.type = TOKEN_TYPE_INTEGER;
    }

    return token;
}

static Token lexer_scan_string(Lexer *lexer, char quote) {
    // starting quote is consumed
    lexer->token_start = lexer->cur;
    lexer->token_start_line = lexer->line;
    lexer->token_start_col = lexer->col;

    while (lexer_peek(lexer) != quote && !lexer_is_eof(lexer))
        lexer_advance(lexer);

    if (lexer_is_eof(lexer)) return lexer_build_error_token(lexer, "unterminated string");

    Token token = lexer_build_token(lexer, TOKEN_TYPE_STRING);
    lexer_advance(lexer); // consume closing quote
    return token;
}

static TokenType check_keyword(const char *s, uint32_t len) {
    for (uint32_t i = 0; i < ARRAY_LEN(keywords); ++i) {
        if (len != keywords[i].len) continue;

        if (string_n_equal(s, keywords[i].str, keywords[i].len))
            return keywords[i].type;
    }
    return TOKEN_TYPE_EOF;
}

static TokenType check_values(const char *s, uint32_t len) {
    for (uint32_t i = 0; i < ARRAY_LEN(values); ++i) {
        if (len != values[i].len) continue;

        if (values[i].ignore_case && string_n_equal_ignore_case(s, values[i].str, values[i].len))
            return values[i].type;
        else if (string_n_equal(s, values[i].str, values[i].len))
            return values[i].type;
    }

    return TOKEN_TYPE_EOF;
}

static Token lexer_scan_comment(Lexer *lexer, bool multi_line) {
    lexer->token_start = lexer->cur;
    lexer->token_start_line = lexer->line;
    lexer->token_start_col = lexer->col;

    if (!multi_line) {
        while (!lexer_is_eof(lexer) && lexer_peek(lexer) != '\n') lexer_advance(lexer);
        Token token = lexer_build_token(lexer, TOKEN_TYPE_SINGLE_LINE_COMMENT);
        lexer_advance(lexer); // consume newline
        return token;
    }

    while (!lexer_is_eof(lexer) && (lexer_peek(lexer) != '*' || lexer_peek_next(lexer) != '/'))
        lexer_advance(lexer);

    if (lexer_is_eof(lexer))
        return lexer_build_error_token(lexer, "unterminated multi-line comment");

    Token token = lexer_build_token(lexer, TOKEN_TYPE_MULTI_LINE_COMMENT);
    lexer_advance(lexer); // consume *
    lexer_advance(lexer); // consume /
    return token;
}

const char *lexer_token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_TYPE_EOF:
            return SNUK_STRINGIFY(TOKEN_TYPE_EOF);
        case TOKEN_TYPE_ERROR:
            return SNUK_STRINGIFY(TOKEN_TYPE_ERROR);
        case TOKEN_TYPE_IDENTIFIER:
            return SNUK_STRINGIFY(TOKEN_TYPE_IDENTIFIER);
        case TOKEN_TYPE_INTEGER:
            return SNUK_STRINGIFY(TOKEN_TYPE_INTEGER);
        case TOKEN_TYPE_FLOAT:
            return SNUK_STRINGIFY(TOKEN_TYPE_FLOAT);
        case TOKEN_TYPE_STRING:
            return SNUK_STRINGIFY(TOKEN_TYPE_STRING);
        case TOKEN_TYPE_TRUE:
            return SNUK_STRINGIFY(TOKEN_TYPE_TRUE);
        case TOKEN_TYPE_FALSE:
            return SNUK_STRINGIFY(TOKEN_TYPE_FALSE);
        case TOKEN_TYPE_NULL:
            return SNUK_STRINGIFY(TOKEN_TYPE_NULL);
        case TOKEN_TYPE_NAN:
            return SNUK_STRINGIFY(TOKEN_TYPE_NAN);
        case TOKEN_TYPE_INF:
            return SNUK_STRINGIFY(TOKEN_TYPE_INF);
        case TOKEN_TYPE_VAR:
            return SNUK_STRINGIFY(TOKEN_TYPE_VAR);
        case TOKEN_TYPE_CONST:
            return SNUK_STRINGIFY(TOKEN_TYPE_CONST);
        case TOKEN_TYPE_IF:
            return SNUK_STRINGIFY(TOKEN_TYPE_IF);
        case TOKEN_TYPE_ELSE:
            return SNUK_STRINGIFY(TOKEN_TYPE_ELSE);
        case TOKEN_TYPE_MATCH:
            return SNUK_STRINGIFY(TOKEN_TYPE_MATCH);
        case TOKEN_TYPE_CASE:
            return SNUK_STRINGIFY(TOKEN_TYPE_CASE);
        case TOKEN_TYPE_WHILE:
            return SNUK_STRINGIFY(TOKEN_TYPE_WHILE);
        case TOKEN_TYPE_DO:
            return SNUK_STRINGIFY(TOKEN_TYPE_DO);
        case TOKEN_TYPE_FOR:
            return SNUK_STRINGIFY(TOKEN_TYPE_FOR);
        case TOKEN_TYPE_RETURN:
            return SNUK_STRINGIFY(TOKEN_TYPE_RETURN);
        case TOKEN_TYPE_BREAK:
            return SNUK_STRINGIFY(TOKEN_TYPE_BREAK);
        case TOKEN_TYPE_CONTINUE:
            return SNUK_STRINGIFY(TOKEN_TYPE_CONTINUE);
        case TOKEN_TYPE_FN:
            return SNUK_STRINGIFY(TOKEN_TYPE_FN);
        case TOKEN_TYPE_PRINT:
            return SNUK_STRINGIFY(TOKEN_TYPE_PRINT);
        case TOKEN_TYPE_SELF:
            return SNUK_STRINGIFY(TOKEN_TYPE_SELF);
        case TOKEN_TYPE_TYPE:
            return SNUK_STRINGIFY(TOKEN_TYPE_TYPE);
        case TOKEN_TYPE_LPAREN:
            return SNUK_STRINGIFY(TOKEN_TYPE_LPAREN);
        case TOKEN_TYPE_RPAREN:
            return SNUK_STRINGIFY(TOKEN_TYPE_RPAREN);
        case TOKEN_TYPE_LBRACE:
            return SNUK_STRINGIFY(TOKEN_TYPE_LBRACE);
        case TOKEN_TYPE_RBRACE:
            return SNUK_STRINGIFY(TOKEN_TYPE_RBRACE);
        case TOKEN_TYPE_LBRACKET:
            return SNUK_STRINGIFY(TOKEN_TYPE_LBRACKET);
        case TOKEN_TYPE_RBRACKET:
            return SNUK_STRINGIFY(TOKEN_TYPE_RBRACKET);
        case TOKEN_TYPE_COMMA:
            return SNUK_STRINGIFY(TOKEN_TYPE_COMMA);
        case TOKEN_TYPE_SEMICOLON:
            return SNUK_STRINGIFY(TOKEN_TYPE_SEMICOLON);
        case TOKEN_TYPE_COLON:
            return SNUK_STRINGIFY(TOKEN_TYPE_COLON);
        case TOKEN_TYPE_DOT:
            return SNUK_STRINGIFY(TOKEN_TYPE_DOT);
        case TOKEN_TYPE_ARROW:
            return SNUK_STRINGIFY(TOKEN_TYPE_ARROW);
        case TOKEN_TYPE_PLUS:
            return SNUK_STRINGIFY(TOKEN_TYPE_PLUS);
        case TOKEN_TYPE_MINUS:
            return SNUK_STRINGIFY(TOKEN_TYPE_MINUS);
        case TOKEN_TYPE_STAR:
            return SNUK_STRINGIFY(TOKEN_TYPE_STAR);
        case TOKEN_TYPE_SLASH:
            return SNUK_STRINGIFY(TOKEN_TYPE_SLASH);
        case TOKEN_TYPE_PERCENT:
            return SNUK_STRINGIFY(TOKEN_TYPE_PERCENT);
        case TOKEN_TYPE_LEFT_SHIFT:
            return SNUK_STRINGIFY(TOKEN_TYPE_LEFT_SHIFT);
        case TOKEN_TYPE_RIGHT_SHIFT:
            return SNUK_STRINGIFY(TOKEN_TYPE_RIGHT_SHIFT);
        case TOKEN_TYPE_OR:
            return SNUK_STRINGIFY(TOKEN_TYPE_OR);
        case TOKEN_TYPE_AND:
            return SNUK_STRINGIFY(TOKEN_TYPE_AND);
        case TOKEN_TYPE_TILDE:
            return SNUK_STRINGIFY(TOKEN_TYPE_TILDE);
        case TOKEN_TYPE_XOR:
            return SNUK_STRINGIFY(TOKEN_TYPE_XOR);
        case TOKEN_TYPE_LOGICAL_AND:
            return SNUK_STRINGIFY(TOKEN_TYPE_LOGICAL_AND);
        case TOKEN_TYPE_LOGICAL_OR:
            return SNUK_STRINGIFY(TOKEN_TYPE_LOGICAL_OR);
        case TOKEN_TYPE_BANG:
            return SNUK_STRINGIFY(TOKEN_TYPE_BANG);
        case TOKEN_TYPE_ASSIGN:
            return SNUK_STRINGIFY(TOKEN_TYPE_ASSIGN);
        case TOKEN_TYPE_PLUS_ASSIGN:
            return SNUK_STRINGIFY(TOKEN_TYPE_PLUS_ASSIGN);
        case TOKEN_TYPE_MINUS_ASSIGN:
            return SNUK_STRINGIFY(TOKEN_TYPE_MINUS_ASSIGN);
        case TOKEN_TYPE_STAR_ASSIGN:
            return SNUK_STRINGIFY(TOKEN_TYPE_STAR_ASSIGN);
        case TOKEN_TYPE_SLASH_ASSIGN:
            return SNUK_STRINGIFY(TOKEN_TYPE_SLASH_ASSIGN);
        case TOKEN_TYPE_PERCENT_ASSIGN:
            return SNUK_STRINGIFY(TOKEN_TYPE_PERCENT_ASSIGN);
        case TOKEN_TYPE_LEFT_SHIFT_ASSIGN:
            return SNUK_STRINGIFY(TOKEN_TYPE_LEFT_SHIFT_ASSIGN);
        case TOKEN_TYPE_RIGHT_SHIFT_ASSIGN:
            return SNUK_STRINGIFY(TOKEN_TYPE_RIGHT_SHIFT_ASSIGN);
        case TOKEN_TYPE_OR_ASSIGN:
            return SNUK_STRINGIFY(TOKEN_TYPE_OR_ASSIGN);
        case TOKEN_TYPE_AND_ASSIGN:
            return SNUK_STRINGIFY(TOKEN_TYPE_AND_ASSIGN);
        case TOKEN_TYPE_XOR_ASSIGN:
            return SNUK_STRINGIFY(TOKEN_TYPE_XOR_ASSIGN);
        case TOKEN_TYPE_EQUAL:
            return SNUK_STRINGIFY(TOKEN_TYPE_EQUAL);
        case TOKEN_TYPE_NOT_EQUAL:
            return SNUK_STRINGIFY(TOKEN_TYPE_NOT_EQUAL);
        case TOKEN_TYPE_LESS_THAN:
            return SNUK_STRINGIFY(TOKEN_TYPE_LESS_THAN);
        case TOKEN_TYPE_GREATER_THAN:
            return SNUK_STRINGIFY(TOKEN_TYPE_GREATER_THAN);
        case TOKEN_TYPE_LESS_THAN_OR_EQUAL:
            return SNUK_STRINGIFY(TOKEN_TYPE_LESS_THAN_OR_EQUAL);
        case TOKEN_TYPE_GREATER_THAN_OR_EQUAL:
            return SNUK_STRINGIFY(TOKEN_TYPE_GREATER_THAN_OR_EQUAL);
        case TOKEN_TYPE_SINGLE_LINE_COMMENT:
            return SNUK_STRINGIFY(TOKEN_TYPE_SINGLE_LINE_COMMENT);
        case TOKEN_TYPE_MULTI_LINE_COMMENT:
            return SNUK_STRINGIFY(TOKEN_TYPE_MULTI_LINE_COMMENT);
        case TOKEN_TYPE_INCREMENT:
            return SNUK_STRINGIFY(TOKEN_TYPE_INCREMENT);
        case TOKEN_TYPE_DECREMENT:
            return SNUK_STRINGIFY(TOKEN_TYPE_DECREMENT);
        case TOKEN_TYPE_MAX:
            return SNUK_STRINGIFY(TOKEN_TYPE_MAX);
        default:
            return "";
    };
}

void lexer_log_token(Token token) {
    log_trace("Token type: %s", lexer_token_type_to_string(token.type));
    if (token.type == TOKEN_TYPE_INTEGER)
        log_trace("\tInteger value: %ld", token.int_value);
    else if (token.type == TOKEN_TYPE_FLOAT)
        log_trace("\tFloat value: %lf", token.float_value);
    else if (token.type == TOKEN_TYPE_ERROR)
        log_trace("\tError line: %.*s and err_msg: %s",
                token.string_value.length, token.string_value.string, token.err_msg);
    else
        log_trace("\tString value: %.*s", token.string_value.length, token.string_value.string);
    log_trace("Line: %d, Column: %d", token.line, token.col);
}

Token lexer_next_token(Lexer *lexer) {
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
            return lexer_build_token(lexer, TOKEN_TYPE_EOF);
        case '(':
            return lexer_build_token(lexer, TOKEN_TYPE_LPAREN);
        case ')':
            return lexer_build_token(lexer, TOKEN_TYPE_RPAREN);
        case '{':
            return lexer_build_token(lexer, TOKEN_TYPE_LBRACE);
        case '}':
            return lexer_build_token(lexer, TOKEN_TYPE_RBRACE);
        case '[':
            return lexer_build_token(lexer, TOKEN_TYPE_LBRACKET);
        case ']':
            return lexer_build_token(lexer, TOKEN_TYPE_RBRACKET);
        case '.':
            return lexer_build_token(lexer, TOKEN_TYPE_DOT);
        case ':':
            return lexer_build_token(lexer, TOKEN_TYPE_COLON);
        case ';':
            return lexer_build_token(lexer, TOKEN_TYPE_SEMICOLON);
        case '+':
            switch (lexer_peek(lexer)) {
                case '+':
                    lexer_advance(lexer);
                    return lexer_build_token(lexer, TOKEN_TYPE_INCREMENT);
                case '=':
                    lexer_advance(lexer);
                    return lexer_build_token(lexer, TOKEN_TYPE_PLUS_ASSIGN);
                default:
                    break;
            }
            return lexer_build_token(lexer, TOKEN_TYPE_PLUS);
        case '-':
            switch (lexer_peek(lexer)) {
                case '>':
                    lexer_advance(lexer);
                    return lexer_build_token(lexer, TOKEN_TYPE_ARROW);
                case '=':
                    lexer_advance(lexer);
                    return lexer_build_token(lexer, TOKEN_TYPE_MINUS_ASSIGN);
                default:
                    break;
            }
            return lexer_build_token(lexer, TOKEN_TYPE_MINUS);
        case '*':
            if (lexer_peek(lexer) == '=') {
                lexer_advance(lexer);
                return lexer_build_token(lexer, TOKEN_TYPE_STAR_ASSIGN);
            }
            return lexer_build_token(lexer, TOKEN_TYPE_STAR);
        case '/':
            switch (lexer_peek(lexer)) {
                case '=':
                    lexer_advance(lexer);
                    return lexer_build_token(lexer, TOKEN_TYPE_SLASH_ASSIGN);
                case '/':
                    lexer_advance(lexer);
                    return lexer_scan_comment(lexer, false);
                case '*':
                    lexer_advance(lexer);
                    return lexer_scan_comment(lexer, true);
                default:
                    break;
            }
            return lexer_build_token(lexer, TOKEN_TYPE_SLASH);
        case '%':
            if (lexer_peek(lexer) == '=') {
                lexer_advance(lexer);
                return lexer_build_token(lexer, TOKEN_TYPE_PERCENT_ASSIGN);
            }
            return lexer_build_token(lexer, TOKEN_TYPE_PERCENT);
        case '<':
            switch (lexer_peek(lexer)) {
                case '<':
                    lexer_advance(lexer);
                    if (lexer_peek(lexer) == '=') {
                        lexer_advance(lexer);
                        return lexer_build_token(lexer, TOKEN_TYPE_LEFT_SHIFT_ASSIGN);
                    }
                    return lexer_build_token(lexer, TOKEN_TYPE_LEFT_SHIFT);
                case '=':
                    lexer_advance(lexer);
                    return lexer_build_token(lexer, TOKEN_TYPE_LESS_THAN_OR_EQUAL);
                default:
                    break;
            }
            return lexer_build_token(lexer, TOKEN_TYPE_LESS_THAN);
        case '>':
            switch (lexer_peek(lexer)) {
                case '>':
                    lexer_advance(lexer);
                    if (lexer_peek(lexer) == '=') {
                        lexer_advance(lexer);
                        return lexer_build_token(lexer, TOKEN_TYPE_RIGHT_SHIFT_ASSIGN);
                    }
                    return lexer_build_token(lexer, TOKEN_TYPE_RIGHT_SHIFT);
                case '=':
                    lexer_advance(lexer);
                    return lexer_build_token(lexer, TOKEN_TYPE_GREATER_THAN_OR_EQUAL);
                default:
                    break;
            }
            return lexer_build_token(lexer, TOKEN_TYPE_GREATER_THAN);
        case '=':
            if (lexer_peek(lexer) == '=') {
                lexer_advance(lexer);
                return lexer_build_token(lexer, TOKEN_TYPE_EQUAL);
            }
            return lexer_build_token(lexer, TOKEN_TYPE_ASSIGN);
        case '!':
            if (lexer_peek(lexer) == '=') {
                lexer_advance(lexer);
                return lexer_build_token(lexer, TOKEN_TYPE_NOT_EQUAL);
            }
            return lexer_build_token(lexer, TOKEN_TYPE_BANG);
        case '&':
            switch (lexer_peek(lexer)) {
                case '&':
                    lexer_advance(lexer);
                    return lexer_build_token(lexer, TOKEN_TYPE_LOGICAL_AND);
                case '=':
                    lexer_advance(lexer);
                    return lexer_build_token(lexer, TOKEN_TYPE_AND_ASSIGN);
                default:
                    break;
            }
            return lexer_build_token(lexer, TOKEN_TYPE_AND);
        case '|':
            switch (lexer_peek(lexer)) {
                case '|':
                    lexer_advance(lexer);
                    return lexer_build_token(lexer, TOKEN_TYPE_LOGICAL_OR);
                case '=':
                    lexer_advance(lexer);
                    return lexer_build_token(lexer, TOKEN_TYPE_OR_ASSIGN);
                default:
                    break;
            }
            return lexer_build_token(lexer, TOKEN_TYPE_OR);
        case '~':
            return lexer_build_token(lexer, TOKEN_TYPE_TILDE);
        case '^':
            if (lexer_peek(lexer) == '=') {
                lexer_advance(lexer);
                return lexer_build_token(lexer, TOKEN_TYPE_XOR_ASSIGN);
            }
            return lexer_build_token(lexer, TOKEN_TYPE_XOR);
        case ',':
            return lexer_build_token(lexer, TOKEN_TYPE_COMMA);

        case '"':
        case '\'':
            return lexer_scan_string(lexer, c);

        default:
            break;
    }

    return lexer_build_error_token(lexer, "unexpected character");
}
