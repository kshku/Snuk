#pragma once

#include "defines.h"

#include "lexer.h"

typedef enum SnukStmtType { SNUK_STMT_EXPR,

    SNUK_STMT_VAR_DECL,
    SNUK_STMT_CONST_DECL,

    SNUK_STMT_IF,
    SNUK_STMT_MATCH,

    SNUK_STMT_WHILE,
    SNUK_STMT_DO_WHILE,
    SNUK_STMT_FOR,

    SNUK_STMT_RETURN,
    SNUK_STMT_BREAK,
    SNUK_STMT_CONTINUE,

    SNUK_STMT_FN,

    SNUK_STMT_TYPE,

    SNUK_STMT_PRINT,

    SNUK_STMT_BLOCK,

    SNUK_STMT_SLCOMMENT,
    SNUK_STMT_MLCOMMENT,

    SNUK_STMT_MAX
} SnukStmtType;

typedef enum SnukExprType {
    SNUK_EXPR_IDENTIFIER,
    SNUK_EXPR_INT_LITERAL,
    SNUK_EXPR_FLOAT_LITERAL,
    SNUK_EXPR_STRING_LITERAL,
    SNUK_EXPR_TRUE_LITERAL,
    SNUK_EXPR_FALSE_LITERAL,
    SNUK_EXPR_NULL_LITERAL,

    SNUK_EXPR_UNARY,
    SNUK_EXPR_BINARY,

    SNUK_EXPR_ASSIGN,

    SNUK_EXPR_CALL,
    SNUK_EXPR_MEMBER,
    SNUK_EXPR_INDEX,

    SNUK_EXPR_MAX
} SnukExprType;

typedef struct SnukExpr SnukExpr;
struct SnukExpr {
    SnukExprType type;

    union {
        struct {
            const char *value;
            uint64_t length;
        } string_literal;

        int64_t int_literal;

        double float_literal;

        struct {
            const char *name;
            uint64_t length;
        } identifier;

        struct {
            SnukTokenType op;
            SnukExpr *operand;
        } unary;

        struct {
            SnukTokenType op;
            SnukExpr *left;
            SnukExpr *right;
        } binary;

        struct {
            SnukExpr *identifier;
            SnukExpr *value;
        } assign;

        struct {
            // TODO:
            SnukExpr **params;
            uint64_t count;
        } call;
    };
};

typedef struct SnukStmt SnukStmt;
struct SnukStmt {
    SnukStmtType type;

    union {
        SnukExpr *expr_stmt;

        struct {
            const char *name;
            uint64_t length;
            SnukExpr *init;
        } decl_stmt; // var or const

        struct {
            SnukExpr * condition;
            SnukStmt *then_branch;
            SnukStmt *else_branch;
        } if_stmt;

        struct {
            SnukExpr *value;
            // TODO:
        } match_stmt;

        struct {
            SnukExpr *condition;
            SnukStmt *block;
        } while_stmt;

        struct {
            SnukStmt *init;
            SnukExpr *cond;
            SnukExpr *update;
            SnukStmt *block;
        } for_stmt;

        SnukExpr *return_stmt;

        struct {
            // TODO:
            SnukStmt *block;
        } fn_stmt;

        struct {
            // TODO:
            bool place_holder;
        } type_stmt;

        struct {
            // darray
            SnukExpr **exprs;
        } print_stmt;

        struct {
            // darray
            SnukStmt **stmts;
        } block_stmt;

        struct {
            const char *comment;
            uint64_t length;
        } comment_stmt;
    };
};

typedef struct SnukParser {
    SnukLexer lexer;
    SnukToken current, previous;

    bool had_error, panic_mode;
} SnukParser;

SNUK_INLINE void snuk_parser_init(SnukParser *parser, const char *src) {
    *parser = (SnukParser){0};
    snuk_lexer_init(&parser->lexer, src);

    parser->previous = (SnukToken){0};
    parser->current = snuk_lexer_next_token(&parser->lexer);
}

SNUK_INLINE void snuk_parser_deinit(SnukParser *parser) {
    if (!parser) return;
    snuk_lexer_deinit(&parser->lexer);
    *parser = (SnukParser){0};
}

SnukStmt *snuk_parser_next_stmt(SnukParser *parser);

const char *snuk_parser_stmt_type_to_string(SnukStmtType type);
const char *snuk_parser_expr_type_to_string(SnukExprType type);
void snuk_parser_log_stmt(SnukStmt *stmt);
void snuk_parser_log_expr(SnukExpr *expr);
