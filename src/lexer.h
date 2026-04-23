#pragma once

#include "defines.h"

#include "string_view.h"

SNUK_STATIC_ASSERT(sizeof(double) == 8, "Expected sizeof(double) to be 8 bytes.");

/**
 * @brief Token categories emitted by the Snuk lexer.
 *
 * Includes literal values, keywords, punctuation, operators, comments, errors,
 * and end-of-file markers.
 */
typedef enum SnukTokenType {
    SNUK_TOKEN_EOF = 0,
    SNUK_TOKEN_ERROR,

    // literals
    SNUK_TOKEN_IDENTIFIER,
    SNUK_TOKEN_INTEGER,
    SNUK_TOKEN_FLOAT,
    SNUK_TOKEN_STRING,

    // keyword literals
    SNUK_TOKEN_TRUE,
    SNUK_TOKEN_FALSE,
    SNUK_TOKEN_NULL,
    SNUK_TOKEN_NAN,
    SNUK_TOKEN_INF,

    // keywords
    SNUK_TOKEN_VAR,
    SNUK_TOKEN_CONST,
    SNUK_TOKEN_ANY,
    SNUK_TOKEN_IF,
    SNUK_TOKEN_ELSE,
    SNUK_TOKEN_MATCH,
    SNUK_TOKEN_CASE,
    SNUK_TOKEN_WHILE,
    SNUK_TOKEN_DO,
    SNUK_TOKEN_FOR,
    SNUK_TOKEN_IN,
    SNUK_TOKEN_RETURN,
    SNUK_TOKEN_BREAK,
    SNUK_TOKEN_CONTINUE,
    SNUK_TOKEN_FN,
    SNUK_TOKEN_TYPE,
    SNUK_TOKEN_SELF,
    SNUK_TOKEN_PRINT,

    // punctuation
    SNUK_TOKEN_LPAREN, // (
    SNUK_TOKEN_RPAREN, // )
    SNUK_TOKEN_LBRACE, // {
    SNUK_TOKEN_RBRACE, // }
    SNUK_TOKEN_LBRACKET, // [
    SNUK_TOKEN_RBRACKET, // ]
    SNUK_TOKEN_COMMA, // ,
    SNUK_TOKEN_SEMICOLON, // ;
    SNUK_TOKEN_COLON, // :
    SNUK_TOKEN_DOT, // .
    SNUK_TOKEN_ARROW, // ->

    // arithmetic operators
    SNUK_TOKEN_PLUS,         // +
    SNUK_TOKEN_MINUS,        // -
    SNUK_TOKEN_STAR,         // *
    SNUK_TOKEN_SLASH,        // /
    SNUK_TOKEN_PERCENT,      // %

    // bitwise operators
    SNUK_TOKEN_AMP,          // &
    SNUK_TOKEN_PIPE,         // |
    SNUK_TOKEN_CARET,        // ^
    SNUK_TOKEN_TILDE,        // ~
    SNUK_TOKEN_LSHIFT,       // <<
    SNUK_TOKEN_RSHIFT,       // >>

    // logical operators
    SNUK_TOKEN_AMP_AMP,      // &&
    SNUK_TOKEN_PIPE_PIPE,    // ||
    SNUK_TOKEN_BANG,         // !
    SNUK_TOKEN_KW_AND,       // and
    SNUK_TOKEN_KW_OR,        // or
    SNUK_TOKEN_KW_NOT,       // not

    // assignment
    SNUK_TOKEN_ASSIGN,       // =

    // compound assignment
    SNUK_TOKEN_PLUS_ASSIGN,  // +=
    SNUK_TOKEN_MINUS_ASSIGN, // -=
    SNUK_TOKEN_STAR_ASSIGN,  // *=
    SNUK_TOKEN_SLASH_ASSIGN, // /=
    SNUK_TOKEN_PERCENT_ASSIGN,  // %=
    SNUK_TOKEN_AMP_ASSIGN,   // &=
    SNUK_TOKEN_PIPE_ASSIGN,  // |=
    SNUK_TOKEN_CARET_ASSIGN, // ^=
    SNUK_TOKEN_LSHIFT_ASSIGN, // <<=
    SNUK_TOKEN_RSHIFT_ASSIGN, // >>=

    // comparison
    SNUK_TOKEN_EQUAL,        // ==
    SNUK_TOKEN_BANG_EQUAL,   // !=
    SNUK_TOKEN_LESS,         // <
    SNUK_TOKEN_GREATER,      // >
    SNUK_TOKEN_LESS_EQUAL,   // <=
    SNUK_TOKEN_GREATER_EQUAL, // >=

    // comments
    SNUK_TOKEN_LINE_COMMENT,   // //
    SNUK_TOKEN_BLOCK_COMMENT,  // /* */

    SNUK_TOKEN_MAX
} SnukTokenType;

/**
 * @brief Lexical token produced from Snuk source text.
 *
 * The token type selects which union member is meaningful: string_literal for
 * textual tokens and error context, int_literal for integer literals, and
 * float_literal for floating-point literals. The line and column fields record
 * the token start using zero-based source coordinates.
 *
 * @note String views point into the source buffer owned by the caller.
 */
typedef struct SnukToken {
    SnukTokenType type;
    union {
        SnukStringView string_literal;
        int64_t int_literal;
        double float_literal;
    };

    const char *err_msg;

    uint64_t line, col;
} SnukToken;

/**
 * @brief Mutable scanner state for a null-terminated Snuk source string.
 *
 * Tracks the original source, the current cursor, the current token start, and
 * zero-based line and column positions for both cursor and token start.
 *
 * @note The source buffer must remain valid for the lifetime of the lexer.
 */
typedef struct SnukLexer {
    const char *src;

    const char *cur;
    const char *token_start;
    uint64_t token_start_line;
    uint64_t token_start_col;

    uint64_t line;
    uint64_t col;
} SnukLexer;

/**
 * @brief Initialize a lexer for the given source string.
 *
 * @param lexer Lexer state to initialize.
 * @param src Null-terminated source buffer to scan.
 *
 * @note The source buffer is borrowed and must outlive all tokens produced by
 * the lexer.
 */
SNUK_INLINE void snuk_lexer_init(SnukLexer *lexer, const char *src) {
    *lexer = (SnukLexer){
        .src = src,
        .cur = src,
        .token_start = src,
        .token_start_line = 0,
        .token_start_col = 0,
        .line = 0,
        .col = 0,
    };
}

/**
 * @brief Reset a lexer to an empty state.
 *
 * @param lexer Lexer state to clear, or NULL.
 */
SNUK_INLINE void snuk_lexer_deinit(SnukLexer *lexer) {
    if (lexer) *lexer = (SnukLexer){0};
}

/**
 * @brief Scan and return the next token from the lexer.
 *
 * Advances the lexer past leading whitespace and the returned token.
 *
 * @param lexer Lexer state to scan from.
 *
 * @return The next token, including comments, errors, or end-of-file.
 */
SnukToken snuk_lexer_next_token(SnukLexer *lexer);

/**
 * @brief Convert a token type to a readable string.
 *
 * @param type Token type to convert.
 *
 * @return String literal naming the token type.
 */
const char *snuk_lexer_token_type_to_string(SnukTokenType type);

/**
 * @brief Log a token for debugging.
 *
 * @param token Token to log.
 */
void snuk_lexer_log_token(SnukToken token);
