#pragma once

#include "defines.h"

SNUK_STATIC_ASSERT(sizeof(double) == 8, "Expected sizeof(double) to be 8 bytes.");

typedef enum SnukTokenType {
    SNUK_TOKEN_EOF = 0,
    SNUK_TOKEN_ERROR,

    // identifiers & literals
    SNUK_TOKEN_IDENTIFIER,
    SNUK_TOKEN_INTEGER,
    SNUK_TOKEN_FLOAT,
    SNUK_TOKEN_STRING,
    SNUK_TOKEN_TRUE,
    SNUK_TOKEN_FALSE,
    SNUK_TOKEN_NULL,
    SNUK_TOKEN_NAN,
    SNUK_TOKEN_INF,

    // keywords
    SNUK_TOKEN_VAR,
    SNUK_TOKEN_CONST,

    SNUK_TOKEN_IF,
    SNUK_TOKEN_ELSE,
    SNUK_TOKEN_MATCH,
    SNUK_TOKEN_CASE,

    SNUK_TOKEN_WHILE,
    SNUK_TOKEN_DO,
    SNUK_TOKEN_FOR,

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
    SNUK_TOKEN_COMMA,
    SNUK_TOKEN_SEMICOLON,
    SNUK_TOKEN_COLON,
    SNUK_TOKEN_DOT,
    SNUK_TOKEN_ARROW,

    // operators
    SNUK_TOKEN_PLUS,
    SNUK_TOKEN_MINUS,
    SNUK_TOKEN_STAR,
    SNUK_TOKEN_SLASH,
    SNUK_TOKEN_PERCENT,

    SNUK_TOKEN_LEFT_SHIFT,
    SNUK_TOKEN_RIGHT_SHIFT,
    SNUK_TOKEN_OR,
    SNUK_TOKEN_AND,
    SNUK_TOKEN_TILDE,
    SNUK_TOKEN_XOR,

    SNUK_TOKEN_LOGICAL_AND,
    SNUK_TOKEN_LOGICAL_OR,
    SNUK_TOKEN_BANG,

    SNUK_TOKEN_ASSIGN,
    // compound assign
    SNUK_TOKEN_PLUS_ASSIGN,
    SNUK_TOKEN_MINUS_ASSIGN,
    SNUK_TOKEN_STAR_ASSIGN,
    SNUK_TOKEN_SLASH_ASSIGN,
    SNUK_TOKEN_PERCENT_ASSIGN,
    SNUK_TOKEN_LEFT_SHIFT_ASSIGN,
    SNUK_TOKEN_RIGHT_SHIFT_ASSIGN,
    SNUK_TOKEN_OR_ASSIGN,
    SNUK_TOKEN_AND_ASSIGN,
    SNUK_TOKEN_XOR_ASSIGN,

    SNUK_TOKEN_INCREMENT,
    SNUK_TOKEN_DECREMENT,

    // relational
    SNUK_TOKEN_EQUAL,
    SNUK_TOKEN_NOT_EQUAL,
    SNUK_TOKEN_LESS_THAN,
    SNUK_TOKEN_GREATER_THAN,
    SNUK_TOKEN_LESS_THAN_OR_EQUAL,
    SNUK_TOKEN_GREATER_THAN_OR_EQUAL,

    SNUK_TOKEN_SLCOMMENT,
    SNUK_TOKEN_MLCOMMENT,

    SNUK_TOKEN_MAX
} SnukTokenType;

typedef struct SnukToken {
    SnukTokenType type;
    union {
        struct {
            const char *string;
            uint32_t length;
        } string_literal;
        int64_t int_literal;
        double float_literal;
    };

    const char *err_msg;

    uint32_t line, col;
} SnukToken;

typedef struct SnukLexer {
    const char *src;

    const char *cur;
    const char *token_start;
    uint32_t token_start_line;
    uint32_t token_start_col;

    uint32_t line;
    uint32_t col;
} SnukLexer;

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

SNUK_INLINE void snuk_lexer_deinit(SnukLexer *lexer) {
    if (lexer) *lexer = (SnukLexer){0};
}

SnukToken snuk_lexer_next_token(SnukLexer *lexer);

const char *snuk_lexer_token_type_to_string(SnukTokenType type);
void snuk_lexer_log_token(SnukToken token);
