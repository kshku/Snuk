#pragma once

#include "defines.h"

#include "parser.h"
#include "darray.h"
#include "memory.h"

typedef enum Precedence {
    PRECEDENCE_NONE = 0,
    PRECEDENCE_ASSIGNMENT, // =
    PRECEDENCE_LOGICAL_OR, // ||
    PRECEDENCE_LOGICAL_AND, // &&
    PRECEDENCE_OR, // |
    PRECEDENCE_XOR, // ^
    PRECEDENCE_AND, // &
    PRECEDENCE_EQUALITY, // == !=
    PRECEDENCE_COMPARISION, // < > <= >=
    PRECEDENCE_SHIFT, // << >>
    PRECEDENCE_TERM, // + -
    PRECEDENCE_FACTOR, // * / %
    PRECEDENCE_UNARY, // - + ~ ! ++ --
    PRECEDENCE_PRIMARY
} Precedence;

typedef SnukExpr *(*prefix_fn)(SnukParser *parser);
typedef SnukExpr *(*infix_fn)(SnukParser *parser, SnukExpr *expr);

typedef struct ParseRule {
    prefix_fn pfn;
    infix_fn ifn;
    Precedence precedence;
} ParseRule;

static SnukExpr *parse_precedence(SnukParser *parser, Precedence precedence);

static SnukExpr *parse_primary(SnukParser *parser);
static SnukExpr *parse_grouping(SnukParser *parser);
static SnukExpr *parse_unary(SnukParser *parser);

static SnukExpr *parse_binary(SnukParser *parser, SnukExpr *left);
static SnukExpr *parse_assignment(SnukParser *parser, SnukExpr *left);

static ParseRule rules[] = {
    [SNUK_TOKEN_IDENTIFIER] = {parse_primary, NULL, PRECEDENCE_NONE},
    [SNUK_TOKEN_INTEGER] = {parse_primary, NULL, PRECEDENCE_NONE},
    [SNUK_TOKEN_FLOAT] = {parse_primary, NULL, PRECEDENCE_NONE},
    [SNUK_TOKEN_STRING] = {parse_primary, NULL, PRECEDENCE_NONE},
    [SNUK_TOKEN_TRUE] = {parse_primary, NULL, PRECEDENCE_NONE},
    [SNUK_TOKEN_FALSE] = {parse_primary, NULL, PRECEDENCE_NONE},
    [SNUK_TOKEN_NULL] = {parse_primary, NULL, PRECEDENCE_NONE},
    [SNUK_TOKEN_NAN] = {parse_primary, NULL, PRECEDENCE_NONE},
    [SNUK_TOKEN_INF] = {parse_primary, NULL, PRECEDENCE_NONE},

    [SNUK_TOKEN_LPAREN] = {parse_grouping, NULL, PRECEDENCE_NONE},

    [SNUK_TOKEN_PLUS] = {parse_unary, parse_binary, PRECEDENCE_TERM},
    [SNUK_TOKEN_MINUS] = {parse_unary, parse_binary, PRECEDENCE_TERM},

    [SNUK_TOKEN_STAR] = {NULL, parse_binary, PRECEDENCE_FACTOR},
    [SNUK_TOKEN_SLASH] = {NULL, parse_binary, PRECEDENCE_FACTOR},
    [SNUK_TOKEN_PERCENT] = {NULL, parse_binary, PRECEDENCE_FACTOR},

    [SNUK_TOKEN_BANG] = {parse_unary, NULL, PRECEDENCE_NONE},
    [SNUK_TOKEN_TILDE] = {parse_unary, NULL, PRECEDENCE_NONE},

    [SNUK_TOKEN_ASSIGN] = {NULL, parse_assignment, PRECEDENCE_ASSIGNMENT},

    [SNUK_TOKEN_EQUAL] = {NULL, parse_binary, PRECEDENCE_EQUALITY},
    [SNUK_TOKEN_NOT_EQUAL] = {NULL, parse_binary, PRECEDENCE_EQUALITY},

    [SNUK_TOKEN_LESS_THAN] = {NULL, parse_binary, PRECEDENCE_COMPARISION},
    [SNUK_TOKEN_LESS_THAN_OR_EQUAL] = {NULL, parse_binary, PRECEDENCE_COMPARISION},
    [SNUK_TOKEN_GREATER_THAN] = {NULL, parse_binary, PRECEDENCE_COMPARISION},
    [SNUK_TOKEN_GREATER_THAN_OR_EQUAL] = {NULL, parse_binary, PRECEDENCE_COMPARISION},

    [SNUK_TOKEN_OR] = {NULL, parse_binary, PRECEDENCE_OR},
    [SNUK_TOKEN_XOR] = {NULL, parse_binary, PRECEDENCE_XOR},
    [SNUK_TOKEN_AND] = {NULL, parse_binary, PRECEDENCE_AND},

    [SNUK_TOKEN_LOGICAL_OR] = {NULL, parse_binary, PRECEDENCE_LOGICAL_OR},
    [SNUK_TOKEN_LOGICAL_AND] = {NULL, parse_binary, PRECEDENCE_LOGICAL_AND},

    [SNUK_TOKEN_LEFT_SHIFT] = {NULL, parse_binary, PRECEDENCE_SHIFT},
    [SNUK_TOKEN_RIGHT_SHIFT] = {NULL, parse_binary, PRECEDENCE_SHIFT},
};

SNUK_INLINE ParseRule *get_rule(SnukTokenType type) {
    return &rules[type];
}

static void parser_error(SnukParser *parser, const char *err_msg);
static void parser_sync(SnukParser *parser);

static SnukExpr *parse_expression(SnukParser *parser);

static SnukStmt *parse_stmt(SnukParser *parser);
static SnukStmt *parse_expr_stmt(SnukParser *parser);
static SnukStmt *parse_decl_stmt(SnukParser *parser, bool is_const);
static SnukStmt *parse_if_stmt(SnukParser *parser);
static SnukStmt *parse_match_stmt(SnukParser *parser);
static SnukStmt *parse_while_stmt(SnukParser *parser);
static SnukStmt *parse_do_while_stmt(SnukParser *parser);
static SnukStmt *parse_for_stmt(SnukParser *parser);
static SnukStmt *parse_flow_stmt(SnukParser *parser);
static SnukStmt *parse_fn_stmt(SnukParser *parser);
static SnukStmt *parse_type_stmt(SnukParser *parser);
static SnukStmt *parse_print_stmt(SnukParser *parser);
static SnukStmt *parse_block_stmt(SnukParser *parser);
static SnukStmt *parse_comment_stmt(SnukParser *parser);

SNUK_INLINE SnukStmt *parser_create_stmt(void) {
    return (SnukStmt *)snuk_alloc(sizeof(SnukStmt), alignof(SnukStmt));
}

SNUK_INLINE SnukStmt *build_expr_stmt(SnukExpr *expr) {
    SnukStmt *expr_stmt = parser_create_stmt();
    *expr_stmt = (SnukStmt){
        .type = SNUK_STMT_EXPR,
        .expr_stmt = expr,
    };
    return expr_stmt;
}

SNUK_INLINE SnukStmt *build_decl_stmt(const char *name, uint64_t length, SnukExpr *init, bool is_const) {
    SnukStmt *decl_stmt = parser_create_stmt();
    *decl_stmt = (SnukStmt){
        .type = is_const ? SNUK_STMT_CONST_DECL : SNUK_STMT_VAR_DECL,
        .decl_stmt = {.name = name, .length = length, .init = init}
    };
    return decl_stmt;
}

SNUK_INLINE SnukStmt *build_if_stmt(SnukExpr *condition, SnukStmt *then_branch, SnukStmt *else_branch) {
    SnukStmt *if_stmt = parser_create_stmt();
    *if_stmt = (SnukStmt){
        .type = SNUK_STMT_IF,
        .if_stmt = {.condition = condition, .then_branch = then_branch, .else_branch = else_branch},
    };
    return if_stmt;
}

SNUK_INLINE SnukStmt *build_while_stmt(SnukExpr *condition, SnukStmt *block, bool is_do_while) {
    SnukStmt *while_stmt = parser_create_stmt();
    *while_stmt = (SnukStmt){
        .type = is_do_while ? SNUK_STMT_DO_WHILE : SNUK_STMT_WHILE,
        .while_stmt = {.condition = condition, .block = block},
    };
    return while_stmt;
}

SNUK_INLINE SnukStmt *build_for_stmt(SnukStmt *init, SnukExpr *cond, SnukExpr *update, SnukStmt *block) {
    SnukStmt *for_stmt = parser_create_stmt();
    *for_stmt = (SnukStmt){
        .type = SNUK_STMT_FOR,
        .for_stmt = {.init = init, .cond = cond, .update = update, .block = block},
    };
    return for_stmt;
}

SNUK_INLINE SnukStmt *build_flow_stmt(SnukTokenType type, SnukExpr *value) {
    SnukStmt *flow_stmt = parser_create_stmt();
    switch (type) {
        case SNUK_TOKEN_RETURN:
            *flow_stmt = (SnukStmt){
                .type = SNUK_STMT_RETURN,
                .return_stmt = value,
            };
            break;
        case SNUK_TOKEN_BREAK:
            *flow_stmt = (SnukStmt){
                .type = SNUK_STMT_BREAK,
            };
            break;
        case SNUK_TOKEN_CONTINUE:
            *flow_stmt = (SnukStmt){
                .type = SNUK_STMT_CONTINUE,
            };
            break;
        default:
            break;
    }
    return flow_stmt;
}

SNUK_INLINE SnukStmt *build_print_stmt(SnukStmt *print_stmt, SnukExpr *expr) {
    if (!print_stmt) {
        print_stmt = parser_create_stmt();
        *print_stmt = (SnukStmt){
            .type = SNUK_STMT_PRINT,
            .print_stmt = {.exprs = snuk_darray_create(SnukExpr *)},
        };
    }
    snuk_darray_push(&print_stmt->print_stmt.exprs, expr);
    return print_stmt;
}

SNUK_INLINE SnukStmt *build_block_stmt(SnukStmt *block_stmt, SnukStmt *stmt) {
    if (!block_stmt) {
        block_stmt = parser_create_stmt();
        *block_stmt = (SnukStmt){
            .type = SNUK_STMT_BLOCK,
            .block_stmt = {.stmts = snuk_darray_create(SnukStmt *)},
        };
    }
    snuk_darray_push(&block_stmt->block_stmt.stmts, stmt);
    return block_stmt;
}

SNUK_INLINE SnukStmt *build_comment_stmt(const char *comment, uint64_t length, bool multi_line) {
    SnukStmt *comment_stmt = parser_create_stmt();
    *comment_stmt = (SnukStmt){
        .type = multi_line ? SNUK_STMT_MLCOMMENT : SNUK_STMT_SLCOMMENT,
        .comment_stmt = {.comment = comment, .length = length},
    };
    return comment_stmt;
}

SNUK_INLINE SnukExpr *parser_create_expr(void) {
    return (SnukExpr *)snuk_alloc(sizeof(SnukExpr), alignof(SnukExpr));
}

SNUK_INLINE SnukExpr *build_null_expr(void) {
    SnukExpr *null_expr = parser_create_expr();
    *null_expr = (SnukExpr){
        .type = SNUK_EXPR_NULL_LITERAL,
    };
    return null_expr;
}

SNUK_INLINE SnukExpr *build_bool_expr(bool expr) {
    SnukExpr *bool_expr = parser_create_expr();
    *bool_expr = (SnukExpr){
        .type = expr ? SNUK_EXPR_TRUE_LITERAL : SNUK_EXPR_FALSE_LITERAL,
    };
    return bool_expr;
}

SNUK_INLINE SnukExpr *build_string_literal_expr(const char *str, uint64_t length) {
    SnukExpr *string_expr = parser_create_expr();
    *string_expr = (SnukExpr){
        .type = SNUK_EXPR_STRING_LITERAL,
        .string_literal = {.value = str, .length = length},
    };
    return string_expr;
}

SNUK_INLINE SnukExpr *build_identifier_expr(const char *name, uint64_t length) {
    SnukExpr *identifier = parser_create_expr();
    *identifier = (SnukExpr){
        .type = SNUK_EXPR_IDENTIFIER,
        .identifier = {.name = name, .length = length},
    };
    return identifier;
}

SNUK_INLINE SnukExpr *build_int_literal_expr(int64_t value) {
    SnukExpr *int_expr = parser_create_expr();
    *int_expr = (SnukExpr){
        .type = SNUK_EXPR_INT_LITERAL,
        .int_literal = value,
    };
    return int_expr;
}

SNUK_INLINE SnukExpr *build_float_literal_expr(double value) {
    SnukExpr *float_expr = parser_create_expr();
    *float_expr = (SnukExpr){
        .type = SNUK_EXPR_FLOAT_LITERAL,
        .float_literal = value,
    };
    return float_expr;
}

SNUK_INLINE SnukExpr *build_unary_expr(SnukTokenType op, SnukExpr *operand) {
    SnukExpr *unary_expr = parser_create_expr();
    *unary_expr = (SnukExpr){
        .type = SNUK_EXPR_UNARY,
        .unary = {.op = op, .operand = operand},
    };
    return unary_expr;
}

SNUK_INLINE SnukExpr *build_binary_expr(SnukTokenType op, SnukExpr *left, SnukExpr *right) {
    SnukExpr *binary_expr = parser_create_expr();
    *binary_expr = (SnukExpr){
        .type = SNUK_EXPR_BINARY,
        .binary = {.op = op, .left = left, .right = right},
    };
    return binary_expr;
}

SNUK_INLINE SnukExpr *build_assign_expr(SnukExpr *identifier, SnukExpr *value) {
    SnukExpr *assign_expr = parser_create_expr();
    *assign_expr = (SnukExpr){
        .type = SNUK_EXPR_ASSIGN,
        .assign = {.identifier = identifier, .value = value},
    };
    return assign_expr;
}

SNUK_INLINE void parser_advance(SnukParser *parser) {
    parser->previous = parser->current;
    parser->current = snuk_lexer_next_token(&parser->lexer);
}

SNUK_INLINE bool parser_check(SnukParser *parser, SnukTokenType expected) {
    // Does not consume
    return parser->current.type == expected;
}

SNUK_INLINE bool parser_match(SnukParser *parser, SnukTokenType expected) {
    // Consumes
    if (!parser_check(parser, expected)) return false;
    parser_advance(parser);
    return true;
}

SNUK_INLINE void parser_expect(SnukParser *parser, SnukTokenType expected, const char *err_msg) {
    if (!parser_match(parser, expected)) parser_error(parser, err_msg);
}
