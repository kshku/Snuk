#pragma once

#include "defines.h"

#include "parser.h"
#include "darray.h"
#include "memory.h"

/**
 * @brief Pratt parser precedence levels.
 */
typedef enum Precedence {
    PRECEDENCE_NONE = 0, /**< No binding precedence. */
    PRECEDENCE_ASSIGNMENT, /**< Assignment precedence for '='. */
    PRECEDENCE_LOGICAL_OR, /**< Logical OR precedence for '||'. */
    PRECEDENCE_LOGICAL_AND, /**< Logical AND precedence for '&&'. */
    PRECEDENCE_OR, /**< Bitwise OR precedence for '|'. */
    PRECEDENCE_XOR, /**< Bitwise XOR precedence for '^'. */
    PRECEDENCE_AND, /**< Bitwise AND precedence for '&'. */
    PRECEDENCE_EQUALITY, /**< Equality precedence for '==' and '!='. */
    PRECEDENCE_COMPARISION, /**< Comparison precedence for '<', '>', '<=', and '>='. */
    PRECEDENCE_SHIFT, /**< Shift precedence for '<<' and '>>'. */
    PRECEDENCE_TERM, /**< Additive precedence for '+' and '-'. */
    PRECEDENCE_FACTOR, /**< Multiplicative precedence for '*', '/', and '%'. */
    PRECEDENCE_UNARY, /**< Unary operator precedence. */
    PRECEDENCE_PRIMARY /**< Primary expression precedence. */
} Precedence;

/**
 * @brief Prefix parse function for tokens that start expressions.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
typedef SnukExpr *(*prefix_fn)(SnukParser *parser);

/**
 * @brief Infix parse function for tokens that continue expressions.
 *
 * @param parser Parser context to operate on.
 * @param expr Left-hand expression already parsed.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
typedef SnukExpr *(*infix_fn)(SnukParser *parser, SnukExpr *expr);

/**
 * @brief Pratt parser rule for a token type.
 */
typedef struct ParseRule {
    prefix_fn pfn; /**< Prefix parse function, or NULL if unsupported. */
    infix_fn ifn; /**< Infix parse function, or NULL if unsupported. */
    Precedence precedence; /**< Binding precedence for the infix function. */
} ParseRule;

/**
 * @brief Parse an expression at or above the given precedence.
 *
 * @param parser Parser context to operate on.
 * @param precedence Minimum precedence to parse.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_precedence(SnukParser *parser, Precedence precedence);

/**
 * @brief Parse a primary expression.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_primary(SnukParser *parser);

/**
 * @brief Parse a grouped expression.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_grouping(SnukParser *parser);

/**
 * @brief Parse a unary expression.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_unary(SnukParser *parser);

/**
 * @brief Parse a binary expression.
 *
 * @param parser Parser context to operate on.
 * @param left Left-hand expression.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_binary(SnukParser *parser, SnukExpr *left);

/**
 * @brief Parse an assignment expression.
 *
 * @param parser Parser context to operate on.
 * @param left Candidate assignment target.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_assignment(SnukParser *parser, SnukExpr *left);

/**
 * @brief Parse an compound assignment expression.
 *
 * @param parser Parser context to operate on.
 * @param left Candidate assignment target.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_compound_assignment(SnukParser *parser, SnukExpr *left);

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

    [SNUK_TOKEN_PLUS_ASSIGN] = {NULL, parse_compound_assignment, PRECEDENCE_ASSIGNMENT},
    [SNUK_TOKEN_MINUS_ASSIGN] = {NULL, parse_compound_assignment, PRECEDENCE_ASSIGNMENT},
    [SNUK_TOKEN_STAR_ASSIGN] = {NULL, parse_compound_assignment, PRECEDENCE_ASSIGNMENT},
    [SNUK_TOKEN_SLASH_ASSIGN] = {NULL, parse_compound_assignment, PRECEDENCE_ASSIGNMENT},
    [SNUK_TOKEN_PERCENT_ASSIGN] = {NULL, parse_compound_assignment, PRECEDENCE_ASSIGNMENT},
    [SNUK_TOKEN_LEFT_SHIFT_ASSIGN] = {NULL, parse_compound_assignment, PRECEDENCE_ASSIGNMENT},
    [SNUK_TOKEN_RIGHT_SHIFT_ASSIGN] = {NULL, parse_compound_assignment, PRECEDENCE_ASSIGNMENT},
    [SNUK_TOKEN_OR_ASSIGN] = {NULL, parse_compound_assignment, PRECEDENCE_ASSIGNMENT},
    [SNUK_TOKEN_AND_ASSIGN] = {NULL, parse_compound_assignment, PRECEDENCE_ASSIGNMENT},
    [SNUK_TOKEN_XOR_ASSIGN] = {NULL, parse_compound_assignment, PRECEDENCE_ASSIGNMENT},

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

/**
 * @brief Get the parse rule for a token type.
 *
 * @param type Token type to look up.
 *
 * @return Pointer to the static parse rule for type.
 */
SNUK_INLINE ParseRule *get_rule(SnukTokenType type) {
    return &rules[type];
}

/**
 * @brief Report a parser error and enter panic mode.
 *
 * @param parser Parser context to operate on.
 * @param err_msg Error message to print.
 */
static void parser_error(SnukParser *parser, const char *err_msg);

/**
 * @brief Recover parser state after an error.
 *
 * @param parser Parser context to operate on.
 */
static void parser_sync(SnukParser *parser);

/**
 * @brief Parse an expression from the lowest precedence.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_expression(SnukParser *parser);

/**
 * @brief Parse the next statement.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed statement, or NULL on parse failure.
 */
static SnukStmt *parse_stmt(SnukParser *parser);

/**
 * @brief Parse an expression statement.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed statement, or NULL on parse failure.
 */
static SnukStmt *parse_expr_stmt(SnukParser *parser);

/**
 * @brief Parse a variable or constant declaration statement.
 *
 * @param parser Parser context to operate on.
 * @param is_const True when parsing a const declaration.
 *
 * @return Parsed statement, or NULL on parse failure.
 */
static SnukStmt *parse_decl_stmt(SnukParser *parser, bool is_const);

/**
 * @brief Parse an if statement.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed statement, or NULL on parse failure.
 */
static SnukStmt *parse_if_stmt(SnukParser *parser);

/**
 * @brief Parse a match statement.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed statement, or NULL on parse failure.
 */
static SnukStmt *parse_match_stmt(SnukParser *parser);

/**
 * @brief Parse a while loop statement.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed statement, or NULL on parse failure.
 */
static SnukStmt *parse_while_stmt(SnukParser *parser);

/**
 * @brief Parse a do-while loop statement.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed statement, or NULL on parse failure.
 */
static SnukStmt *parse_do_while_stmt(SnukParser *parser);

/**
 * @brief Parse a for loop statement.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed statement, or NULL on parse failure.
 */
static SnukStmt *parse_for_stmt(SnukParser *parser);

/**
 * @brief Parse return, break, or continue statements.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed statement, or NULL on parse failure.
 */
static SnukStmt *parse_flow_stmt(SnukParser *parser);

/**
 * @brief Parse a function declaration statement.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed statement, or NULL on parse failure.
 */
static SnukStmt *parse_fn_stmt(SnukParser *parser);

/**
 * @brief Parse a type declaration statement.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed statement, or NULL on parse failure.
 */
static SnukStmt *parse_type_stmt(SnukParser *parser);

/**
 * @brief Parse a print statement.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed statement, or NULL on parse failure.
 */
static SnukStmt *parse_print_stmt(SnukParser *parser);

/**
 * @brief Parse a block statement.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed statement, or NULL on parse failure.
 */
static SnukStmt *parse_block_stmt(SnukParser *parser);

/**
 * @brief Parse a comment statement.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed statement, or NULL on parse failure.
 */
static SnukStmt *parse_comment_stmt(SnukParser *parser);

/**
 * @brief Allocate a statement node.
 *
 * @param parser Parser context to operate on.
 *
 * @return Newly allocated statement storage.
 */
SNUK_INLINE SnukStmt *parser_create_stmt(SnukParser *parser) {
    return (SnukStmt *)parser->alloc(parser->alloc_data,
            sizeof(SnukStmt), alignof(SnukStmt));
}

/**
 * @brief Allocate an expression node.
 *
 * @param parser Parser context to operate on.
 *
 * @return Newly allocated expression storage.
 */
SNUK_INLINE SnukExpr *parser_create_expr(SnukParser *parser) {
    return (SnukExpr *)parser->alloc(parser->alloc_data,
            sizeof(SnukExpr), alignof(SnukExpr));
}

/**
 * @brief Allocate a parameter node.
 *
 * @param parser Parser context to operate on.
 *
 * @return Newly allocated parameter storage.
 */
SNUK_INLINE SnukParam *parser_create_param(SnukParser *parser) {
    return (SnukParam *)parser->alloc(parser->alloc_data,
            sizeof(SnukParam), alignof(SnukParam));
}

/**
 * @brief Build an expression statement node.
 *
 * @param parser Parser context to operate on.
 * @param expr Expression payload.
 *
 * @return Newly allocated expression statement node.
 */
SNUK_INLINE SnukStmt *build_expr_stmt(SnukParser *parser, SnukExpr *expr) {
    SnukStmt *expr_stmt = parser_create_stmt(parser);
    *expr_stmt = (SnukStmt){
        .type = SNUK_STMT_EXPR,
        .expr_stmt = expr,
    };
    return expr_stmt;
}

/**
 * @brief Build a variable or constant declaration statement node.
 *
 * @param parser Parser context to operate on.
 * @param identifier Declared identifier expression.
 * @param type Type of the variable or constant.
 * @param init Initializer expression.
 * @param is_const True to build a const declaration.
 *
 * @return Newly allocated declaration statement node.
 */
SNUK_INLINE SnukStmt *build_decl_stmt(SnukParser *parser, SnukExpr *identifier, SnukExpr *type, SnukExpr *init, bool is_const) {
    SnukStmt *decl_stmt = parser_create_stmt(parser);
    *decl_stmt = (SnukStmt){
        .type = is_const ? SNUK_STMT_CONST_DECL : SNUK_STMT_VAR_DECL,
        .decl_stmt = {.identifier = identifier, .type = type, .init = init},
    };
    return decl_stmt;
}

/**
 * @brief Build an if statement node.
 *
 * @param parser Parser context to operate on.
 * @param condition Condition expression.
 * @param then_branch Statement executed when condition is true.
 * @param else_branch Optional statement executed otherwise.
 *
 * @return Newly allocated if statement node.
 */
SNUK_INLINE SnukStmt *build_if_stmt(SnukParser *parser, SnukExpr *condition, SnukStmt *then_branch, SnukStmt *else_branch) {
    SnukStmt *if_stmt = parser_create_stmt(parser);
    *if_stmt = (SnukStmt){
        .type = SNUK_STMT_IF,
        .if_stmt = {
            .condition = condition,
            .then_branch = then_branch,
            .else_branch = else_branch,
        },
    };
    return if_stmt;
}

/**
 * @brief Build a while or do-while statement node.
 *
 * @param parser Parser context to operate on.
 * @param condition Loop condition expression.
 * @param block Loop body block.
 * @param is_do_while True to build a do-while statement.
 *
 * @return Newly allocated loop statement node.
 */
SNUK_INLINE SnukStmt *build_while_stmt(SnukParser *parser, SnukExpr *condition, SnukStmt *block, bool is_do_while) {
    SnukStmt *while_stmt = parser_create_stmt(parser);
    *while_stmt = (SnukStmt){
        .type = is_do_while ? SNUK_STMT_DO_WHILE : SNUK_STMT_WHILE,
        .while_stmt = {.condition = condition, .block = block},
    };
    return while_stmt;
}

/**
 * @brief Build a for loop statement node.
 *
 * @param parser Parser context to operate on.
 * @param init Optional initializer statement.
 * @param cond Optional condition expression.
 * @param update Optional update expression.
 * @param block Loop body block.
 *
 * @return Newly allocated for statement node.
 */
SNUK_INLINE SnukStmt *build_for_stmt(SnukParser *parser, SnukStmt *init, SnukExpr *cond, SnukExpr *update, SnukStmt *block) {
    SnukStmt *for_stmt = parser_create_stmt(parser);
    *for_stmt = (SnukStmt){
        .type = SNUK_STMT_FOR,
        .for_stmt = {.init = init, .cond = cond, .update = update, .block = block},
    };
    return for_stmt;
}

/**
 * @brief Build a control-flow statement node.
 *
 * @param parser Parser context to operate on.
 * @param type Token type for the control-flow keyword.
 * @param value Optional return value expression.
 *
 * @return Newly allocated control-flow statement node.
 */
SNUK_INLINE SnukStmt *build_flow_stmt(SnukParser *parser, SnukTokenType type, SnukExpr *value) {
    SnukStmt *flow_stmt = parser_create_stmt(parser);
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

/**
 * @brief Build a function declaration statement node.
 *
 * @param parser Parser context to operate on.
 * @param identifier Function name expression.
 * @param params Dynamic array of parameter nodes.
 * @param body Function body block.
 *
 * @return Newly allocated function statement node.
 */
SNUK_INLINE SnukStmt *build_fn_stmt(SnukParser *parser, SnukExpr *identifier, SnukParam **params, SnukStmt *body) {
    SnukStmt *fn_stmt = parser_create_stmt(parser);
    *fn_stmt = (SnukStmt){
        .type = SNUK_STMT_FN,
        .fn_stmt = {
            .identifier = identifier,
            .params = params,
            .body = body,
        },
    };
    return fn_stmt;
}

/**
 * @brief Build a type declaration statement node.
 *
 * @param parser Parser context to operate on.
 * @param identifier Type name expression.
 * @param vars Dynamic array of field declarations.
 * @param fns Dynamic array of method declarations.
 *
 * @return Newly allocated type statement node.
 */
SNUK_INLINE SnukStmt *build_type_stmt(SnukParser *parser, SnukExpr *identifier, SnukStmt **vars, SnukStmt **fns) {
    SnukStmt *type_stmt = parser_create_stmt(parser);
    *type_stmt = (SnukStmt){
        .type = SNUK_STMT_TYPE,
        .type_stmt = {
            .identifier = identifier,
            .vars = vars,
            .fns = fns,
        },
    };
    return type_stmt;
}

/**
 * @brief Build or append to a print statement node.
 *
 * @param parser Parser context to operate on.
 * @param print_stmt Existing print statement to append to, or NULL to create one.
 * @param expr Expression to append.
 *
 * @return Print statement node.
 */
SNUK_INLINE SnukStmt *build_print_stmt(SnukParser *parser, SnukStmt *print_stmt, SnukExpr *expr) {
    if (!print_stmt) {
        print_stmt = parser_create_stmt(parser);
        *print_stmt = (SnukStmt){
            .type = SNUK_STMT_PRINT,
            .print_stmt = {.exprs = snuk_darray_create(SnukExpr *)},
        };
    }
    if (expr) snuk_darray_push(&print_stmt->print_stmt.exprs, expr);
    return print_stmt;
}

/**
 * @brief Build or append to a block statement node.
 *
 * @param parser Parser context to operate on.
 * @param block_stmt Existing block statement to append to, or NULL to create one.
 * @param stmt Statement to append.
 *
 * @return Block statement node.
 */
SNUK_INLINE SnukStmt *build_block_stmt(SnukParser *parser, SnukStmt *block_stmt, SnukStmt *stmt) {
    if (!block_stmt) {
        block_stmt = parser_create_stmt(parser);
        *block_stmt = (SnukStmt){
            .type = SNUK_STMT_BLOCK,
            .block_stmt = {.stmts = snuk_darray_create(SnukStmt *)},
        };
    }
    if (stmt) snuk_darray_push(&block_stmt->block_stmt.stmts, stmt);
    return block_stmt;
}

/**
 * @brief Build a comment statement node from a comment token.
 *
 * @param parser Parser context to operate on.
 * @param comment_token Source comment token.
 *
 * @return Newly allocated comment statement node.
 */
SNUK_INLINE SnukStmt *build_comment_stmt(SnukParser *parser, SnukToken comment_token) {
    SnukStmt *comment_stmt = parser_create_stmt(parser);
    *comment_stmt = (SnukStmt){
        .type = comment_token.type == SNUK_TOKEN_MLCOMMENT ? SNUK_STMT_MLCOMMENT : SNUK_STMT_SLCOMMENT,
        .comment = comment_token.string_literal,
    };
    return comment_stmt;
}

/**
 * @brief Build a null literal expression node.
 *
 * @param parser Parser context to operate on.
 *
 * @return Newly allocated null expression node.
 */
SNUK_INLINE SnukExpr *build_null_expr(SnukParser *parser) {
    SnukExpr *null_expr = parser_create_expr(parser);
    *null_expr = (SnukExpr){
        .type = SNUK_EXPR_NULL_LITERAL,
    };
    return null_expr;
}

/**
 * @brief Build a boolean literal expression node.
 *
 * @param parser Parser context to operate on.
 *
 * @return Newly allocated boolean expression node.
 */
SNUK_INLINE SnukExpr *build_bool_expr(SnukParser *parser) {
    SnukExpr *bool_expr = parser_create_expr(parser);
    *bool_expr = (SnukExpr){
        .type = parser->previous.type == SNUK_TOKEN_TRUE
            ? SNUK_EXPR_TRUE_LITERAL : SNUK_EXPR_FALSE_LITERAL,
    };
    return bool_expr;
}

/**
 * @brief Build a string literal expression node.
 *
 * @param parser Parser context to operate on.
 *
 * @return Newly allocated string literal expression node.
 */
SNUK_INLINE SnukExpr *build_string_literal_expr(SnukParser *parser) {
    SnukExpr *string_expr = parser_create_expr(parser);
    *string_expr = (SnukExpr){
        .type = SNUK_EXPR_STRING_LITERAL,
        .string_literal = parser->previous.string_literal,
    };
    return string_expr;
}

/**
 * @brief Build an identifier expression node.
 *
 * @param parser Parser context to operate on.
 *
 * @return Newly allocated identifier expression node.
 */
SNUK_INLINE SnukExpr *build_identifier_expr(SnukParser *parser) {
    SnukExpr *identifier = parser_create_expr(parser);
    *identifier = (SnukExpr){
        .type = SNUK_EXPR_IDENTIFIER,
        .identifier = parser->previous.string_literal,
    };
    return identifier;
}

/**
 * @brief Build an integer literal expression node.
 *
 * @param parser Parser context to operate on.
 *
 * @return Newly allocated integer literal expression node.
 */
SNUK_INLINE SnukExpr *build_int_literal_expr(SnukParser *parser) {
    SnukExpr *int_expr = parser_create_expr(parser);
    *int_expr = (SnukExpr){
        .type = SNUK_EXPR_INT_LITERAL,
        .int_literal = parser->previous.int_literal,
    };
    return int_expr;
}

/**
 * @brief Build a floating-point literal expression node.
 *
 * @param parser Parser context to operate on.
 *
 * @return Newly allocated floating-point literal expression node.
 */
SNUK_INLINE SnukExpr *build_float_literal_expr(SnukParser *parser) {
    SnukExpr *float_expr = parser_create_expr(parser);
    *float_expr = (SnukExpr){
        .type = SNUK_EXPR_FLOAT_LITERAL,
        .float_literal = parser->previous.float_literal,
    };
    return float_expr;
}

/**
 * @brief Build a unary expression node.
 *
 * @param parser Parser context to operate on.
 * @param op Unary operator token type.
 * @param operand Operand expression.
 *
 * @return Newly allocated unary expression node.
 */
SNUK_INLINE SnukExpr *build_unary_expr(SnukParser *parser, SnukTokenType op, SnukExpr *operand) {
    SnukExpr *unary_expr = parser_create_expr(parser);
    *unary_expr = (SnukExpr){
        .type = SNUK_EXPR_UNARY,
        .unary = {.op = op, .operand = operand},
    };
    return unary_expr;
}

/**
 * @brief Build a binary expression node.
 *
 * @param parser Parser context to operate on.
 * @param op Binary operator token type.
 * @param left Left-hand operand expression.
 * @param right Right-hand operand expression.
 *
 * @return Newly allocated binary expression node.
 */
SNUK_INLINE SnukExpr *build_binary_expr(SnukParser *parser, SnukTokenType op, SnukExpr *left, SnukExpr *right) {
    SnukExpr *binary_expr = parser_create_expr(parser);
    *binary_expr = (SnukExpr){
        .type = SNUK_EXPR_BINARY,
        .binary = {.op = op, .left = left, .right = right},
    };
    return binary_expr;
}

/**
 * @brief Build an assignment expression node.
 *
 * @param parser Parser context to operate on.
 * @param identifier Assignment target identifier expression.
 * @param value Assigned value expression.
 *
 * @return Newly allocated assignment expression node.
 */
SNUK_INLINE SnukExpr *build_assign_expr(SnukParser *parser, SnukExpr *identifier, SnukExpr *value) {
    SnukExpr *assign_expr = parser_create_expr(parser);
    *assign_expr = (SnukExpr){
        .type = SNUK_EXPR_ASSIGN,
        .assign = {.identifier = identifier, .value = value},
    };
    return assign_expr;
}

/**
 * @brief Build an compound assignment expression node.
 *
 * @param parser Parser context to operate on.
 * @param op The compound operator
 * @param identifier Assignment target identifier expression.
 * @param value Assigned value expression.
 *
 * @return Newly allocated assignment expression node.
 */
SNUK_INLINE SnukExpr *build_compound_assign_expr(SnukParser *parser, SnukTokenType op, SnukExpr *identifier, SnukExpr *value) {
    SnukExpr *compound = parser_create_expr(parser);
    *compound = (SnukExpr){
        .type = SNUK_EXPR_COMPOUND_ASSIGN,
        .compound_assign = {.op = op, .identifier = identifier, .value = value},
    };
    return compound;
}

/**
 * @brief Build a function parameter node.
 *
 * @param parser Parser context to operate on.
 * @param identifier Parameter name expression.
 * @param type Type of the parameter
 * @param default_value Optional default value expression.
 *
 * @return Newly allocated parameter node.
 */
SNUK_INLINE SnukParam *build_param(SnukParser *parser, SnukExpr *identifier, SnukExpr *type, SnukExpr *default_value) {
    SnukParam *param = parser_create_param(parser);
    *param = (SnukParam){
        .identifier = identifier,
        .type = type,
        .default_value = default_value,
    };
    return param;
}

/**
 * @brief Advance to the next token.
 *
 * @param parser Parser context to operate on.
 */
SNUK_INLINE void parser_advance(SnukParser *parser) {
    parser->previous = parser->current;
    parser->current = snuk_lexer_next_token(&parser->lexer);
}

/**
 * @brief Check whether the current token has the expected type.
 *
 * @param parser Parser context to operate on.
 * @param expected Expected token type.
 *
 * @return True when the current token matches expected.
 */
SNUK_INLINE bool parser_check(SnukParser *parser, SnukTokenType expected) {
    // Does not consume
    return parser->current.type == expected;
}

/**
 * @brief Consume the current token if it has the expected type.
 *
 * @param parser Parser context to operate on.
 * @param expected Expected token type.
 *
 * @return True when a token was consumed.
 */
SNUK_INLINE bool parser_match(SnukParser *parser, SnukTokenType expected) {
    // Consumes
    if (!parser_check(parser, expected)) return false;
    parser_advance(parser);
    return true;
}

/**
 * @brief Require and consume a token of the expected type.
 *
 * @param parser Parser context to operate on.
 * @param expected Expected token type.
 * @param err_msg Error message to report if the token does not match.
 */
SNUK_INLINE void parser_expect(SnukParser *parser, SnukTokenType expected, const char *err_msg) {
    if (!parser_match(parser, expected)) parser_error(parser, err_msg);
}
