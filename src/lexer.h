#pragma once

#include "defines.h"

SNUK_STATIC_ASSERT(sizeof(double) == 8, "Expected sizeof(double) to be 8 bytes.");

typedef enum TokenType {
    TOKEN_TYPE_EOF = 0,
    TOKEN_TYPE_ERROR,

    // identifiers & literals
    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_INTEGER,
    TOKEN_TYPE_FLOAT,
    TOKEN_TYPE_STRING,
    TOKEN_TYPE_TRUE,
    TOKEN_TYPE_FALSE,
    TOKEN_TYPE_NULL,
    TOKEN_TYPE_NAN,
    TOKEN_TYPE_INF,

    // keywords
    TOKEN_TYPE_VAR,
    TOKEN_TYPE_CONST,
    TOKEN_TYPE_IF,
    TOKEN_TYPE_ELSE,
    TOKEN_TYPE_MATCH,
    TOKEN_TYPE_CASE,
    TOKEN_TYPE_WHILE,
    TOKEN_TYPE_DO,
    TOKEN_TYPE_FOR,
    TOKEN_TYPE_RETURN,
    TOKEN_TYPE_BREAK,
    TOKEN_TYPE_CONTINUE,
    TOKEN_TYPE_FN,
    TOKEN_TYPE_PRINT,
    TOKEN_TYPE_SELF,
    TOKEN_TYPE_TYPE,

    // punctuation
    TOKEN_TYPE_LPAREN, // (
    TOKEN_TYPE_RPAREN, // )
    TOKEN_TYPE_LBRACE, // {
    TOKEN_TYPE_RBRACE, // }
    TOKEN_TYPE_LBRACKET, // [
    TOKEN_TYPE_RBRACKET, // ]
    TOKEN_TYPE_COMMA,
    TOKEN_TYPE_SEMICOLON,
    TOKEN_TYPE_COLON,
    TOKEN_TYPE_DOT,
    TOKEN_TYPE_ARROW,

    // operators
    TOKEN_TYPE_PLUS,
    TOKEN_TYPE_MINUS,
    TOKEN_TYPE_STAR,
    TOKEN_TYPE_SLASH,
    TOKEN_TYPE_PERCENT,

    TOKEN_TYPE_LEFT_SHIFT,
    TOKEN_TYPE_RIGHT_SHIFT,
    TOKEN_TYPE_OR,
    TOKEN_TYPE_AND,
    TOKEN_TYPE_TILDE,
    TOKEN_TYPE_XOR,

    TOKEN_TYPE_LOGICAL_AND,
    TOKEN_TYPE_LOGICAL_OR,
    TOKEN_TYPE_BANG,

    TOKEN_TYPE_ASSIGN,
    // compound assign
    TOKEN_TYPE_PLUS_ASSIGN,
    TOKEN_TYPE_MINUS_ASSIGN,
    TOKEN_TYPE_STAR_ASSIGN,
    TOKEN_TYPE_SLASH_ASSIGN,
    TOKEN_TYPE_PERCENT_ASSIGN,
    TOKEN_TYPE_LEFT_SHIFT_ASSIGN,
    TOKEN_TYPE_RIGHT_SHIFT_ASSIGN,
    TOKEN_TYPE_OR_ASSIGN,
    TOKEN_TYPE_AND_ASSIGN,
    TOKEN_TYPE_XOR_ASSIGN,

    // relational
    TOKEN_TYPE_EQUAL,
    TOKEN_TYPE_NOT_EQUAL,
    TOKEN_TYPE_LESS_THAN,
    TOKEN_TYPE_GREATER_THAN,
    TOKEN_TYPE_LESS_THAN_OR_EQUAL,
    TOKEN_TYPE_GREATER_THAN_OR_EQUAL,

    TOKEN_TYPE_SINGLE_LINE_COMMENT,
    TOKEN_TYPE_MULTI_LINE_COMMENT,

    TOKEN_TYPE_INCREMENT,
    TOKEN_TYPE_DECREMENT,

    TOKEN_TYPE_MAX
} TokenType;

typedef struct Token {
    TokenType type;
    union {
        struct {
            const char *string;
            uint32_t length;
        } string_value;
        int64_t int_value;
        double float_value;
    };

    const char *err_msg;

    uint32_t line, col;
} Token;

typedef struct Lexer {
    const char *src;

    const char *cur;
    const char *token_start;
    uint32_t token_start_line;
    uint32_t token_start_col;

    uint32_t line;
    uint32_t col;
} Lexer;

SNUK_INLINE void lexer_init(Lexer *lexer, const char *src) {
    *lexer = (Lexer){
        .src = src,
        .cur = src,
        .token_start = src,
        .token_start_line = 0,
        .token_start_col = 0,
        .line = 0,
        .col = 0,
    };
}

SNUK_INLINE void lexer_deinit(Lexer *lexer) {
    if (lexer) *lexer = (Lexer){0};
}

SNUK_INLINE bool lexer_is_eof(Lexer *lexer) {
    return *lexer->cur == '\0';
}

Token lexer_next_token(Lexer *lexer);

const char *lexer_token_type_to_string(TokenType type);
void lexer_log_token(Token token);
