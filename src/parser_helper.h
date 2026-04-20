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

/**
 * @brief Parse an if expression.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_if(SnukParser *parser);

/**
 * @brief Parse an match expression.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_match(SnukParser *parser);

/**
 * @brief Parse an while or do while loop expression.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_while(SnukParser *parser);

/**
 * @brief Parse an for loop expression.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_for(SnukParser *parser);

/**
 * @brief Parse an function expression.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_fn(SnukParser *parser);

/**
 * @brief Parse an type expression.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_type(SnukParser *parser);

/**
 * @brief Parse an block expression.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
static SnukExpr *parse_block(SnukParser *parser);

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
    [SNUK_TOKEN_KW_NOT] = {parse_unary, NULL, PRECEDENCE_NONE},
    [SNUK_TOKEN_TILDE] = {parse_unary, NULL, PRECEDENCE_NONE},

    [SNUK_TOKEN_ASSIGN] = {NULL, parse_assignment, PRECEDENCE_ASSIGNMENT},

    [SNUK_TOKEN_PLUS_ASSIGN] = {NULL, parse_compound_assignment, PRECEDENCE_ASSIGNMENT},
    [SNUK_TOKEN_MINUS_ASSIGN] = {NULL, parse_compound_assignment, PRECEDENCE_ASSIGNMENT},
    [SNUK_TOKEN_STAR_ASSIGN] = {NULL, parse_compound_assignment, PRECEDENCE_ASSIGNMENT},
    [SNUK_TOKEN_SLASH_ASSIGN] = {NULL, parse_compound_assignment, PRECEDENCE_ASSIGNMENT},
    [SNUK_TOKEN_PERCENT_ASSIGN] = {NULL, parse_compound_assignment, PRECEDENCE_ASSIGNMENT},
    [SNUK_TOKEN_LSHIFT_ASSIGN] = {NULL, parse_compound_assignment, PRECEDENCE_ASSIGNMENT},
    [SNUK_TOKEN_RSHIFT_ASSIGN] = {NULL, parse_compound_assignment, PRECEDENCE_ASSIGNMENT},
    [SNUK_TOKEN_PIPE_ASSIGN] = {NULL, parse_compound_assignment, PRECEDENCE_ASSIGNMENT},
    [SNUK_TOKEN_AMP_ASSIGN] = {NULL, parse_compound_assignment, PRECEDENCE_ASSIGNMENT},
    [SNUK_TOKEN_CARET_ASSIGN] = {NULL, parse_compound_assignment, PRECEDENCE_ASSIGNMENT},

    [SNUK_TOKEN_EQUAL] = {NULL, parse_binary, PRECEDENCE_EQUALITY},
    [SNUK_TOKEN_BANG_EQUAL] = {NULL, parse_binary, PRECEDENCE_EQUALITY},

    [SNUK_TOKEN_LESS] = {NULL, parse_binary, PRECEDENCE_COMPARISION},
    [SNUK_TOKEN_LESS_EQUAL] = {NULL, parse_binary, PRECEDENCE_COMPARISION},
    [SNUK_TOKEN_GREATER] = {NULL, parse_binary, PRECEDENCE_COMPARISION},
    [SNUK_TOKEN_GREATER_EQUAL] = {NULL, parse_binary, PRECEDENCE_COMPARISION},

    [SNUK_TOKEN_PIPE] = {NULL, parse_binary, PRECEDENCE_OR},
    [SNUK_TOKEN_CARET] = {NULL, parse_binary, PRECEDENCE_XOR},
    [SNUK_TOKEN_AMP] = {NULL, parse_binary, PRECEDENCE_AND},

    [SNUK_TOKEN_PIPE_PIPE] = {NULL, parse_binary, PRECEDENCE_LOGICAL_OR},
    [SNUK_TOKEN_KW_OR] = {NULL, parse_binary, PRECEDENCE_LOGICAL_OR},
    [SNUK_TOKEN_AMP_AMP] = {NULL, parse_binary, PRECEDENCE_LOGICAL_AND},
    [SNUK_TOKEN_KW_AND] = {NULL, parse_binary, PRECEDENCE_LOGICAL_AND},

    [SNUK_TOKEN_LSHIFT] = {NULL, parse_binary, PRECEDENCE_SHIFT},
    [SNUK_TOKEN_RSHIFT] = {NULL, parse_binary, PRECEDENCE_SHIFT},

    [SNUK_TOKEN_IF] = {parse_if, NULL, PRECEDENCE_NONE},
    [SNUK_TOKEN_MATCH] = {parse_match, NULL, PRECEDENCE_NONE},

    [SNUK_TOKEN_WHILE] = {parse_while, NULL, PRECEDENCE_NONE},
    [SNUK_TOKEN_DO] = {parse_while, NULL, PRECEDENCE_NONE},

    [SNUK_TOKEN_FOR] = {parse_for, NULL, PRECEDENCE_NONE},

    [SNUK_TOKEN_FN] = {parse_fn, NULL, PRECEDENCE_NONE},
    [SNUK_TOKEN_TYPE] = {parse_type, NULL, PRECEDENCE_NONE},

    [SNUK_TOKEN_LBRACE] = {parse_block, NULL, PRECEDENCE_NONE},
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
 * @brief Parse the next item.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed item, or NULL on parse failure.
 */
static SnukItem *parse_item(SnukParser *parser);

/**
 * @brief Parse an expression item.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed item, or NULL on parse failure.
 */
static SnukItem *parse_expr_item(SnukParser *parser);

/**
 * @brief Parse a variable or constant declaration item.
 *
 * @param parser Parser context to operate on.
 * @param is_const True when parsing a const declaration.
 *
 * @return Parsed item, or NULL on parse failure.
 */
static SnukItem *parse_decl_item(SnukParser *parser, bool is_const);

/**
 * @brief Parse return, break, or continue items.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed item, or NULL on parse failure.
 */
static SnukItem *parse_flow_item(SnukParser *parser);

/**
 * @brief Parse a function declaration item.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed item, or NULL on parse failure.
 */
static SnukItem *parse_fn_item(SnukParser *parser);

/**
 * @brief Parse a type declaration item.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed item, or NULL on parse failure.
 */
static SnukItem *parse_type_item(SnukParser *parser);

/**
 * @brief Parse a print item.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed item, or NULL on parse failure.
 */
static SnukItem *parse_print_item(SnukParser *parser);

/**
 * @brief Parse a comment item.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed item, or NULL on parse failure.
 */
static SnukItem *parse_comment_item(SnukParser *parser);

/**
 * @brief Allocate a item.
 *
 * @param parser Parser context to operate on.
 *
 * @return Newly allocated item storage.
 */
SNUK_INLINE SnukItem *parser_create_item(SnukParser *parser) {
    return (SnukItem *)parser->alloc(parser->alloc_data,
            sizeof(SnukItem), alignof(SnukItem));
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
 * @brief Build an expression item.
 *
 * @param parser Parser context to operate on.
 * @param expr Expression payload.
 *
 * @return Newly allocated expression item.
 */
SNUK_INLINE SnukItem *build_expr_item(SnukParser *parser, SnukExpr *expr) {
    SnukItem *item = parser_create_item(parser);
    *item = (SnukItem){
        .type = SNUK_ITEM_EXPR,
        .expr = expr,
    };
    return item;
}

/**
 * @brief Build a variable or constant declaration item.
 *
 * @param parser Parser context to operate on.
 * @param identifier Declared identifier expression.
 * @param type Type of the variable or constant.
 * @param init Initializer expression.
 * @param is_const True to build a const declaration.
 *
 * @return Newly allocated declaration item.
 */
SNUK_INLINE SnukItem *build_decl_item(SnukParser *parser, SnukExpr *identifier, SnukExpr *type, SnukExpr *init, bool is_const) {
    SnukItem *item = parser_create_item(parser);
    *item = (SnukItem){
        .type = is_const ? SNUK_ITEM_CONST_DECL : SNUK_ITEM_VAR_DECL,
        .var_decl = {.identifier = identifier, .type = type, .init = init},
    };
    return item;
}

/**
 * @brief Build a control-flow item.
 *
 * @param parser Parser context to operate on.
 * @param type Token type for the control-flow keyword.
 * @param value Optional return value expression.
 *
 * @return Newly allocated control-flow item.
 */
SNUK_INLINE SnukItem *build_flow_item(SnukParser *parser, SnukTokenType type, SnukExpr *value) {
    SnukItem *item = parser_create_item(parser);
    switch (type) {
        case SNUK_TOKEN_RETURN:
            *item = (SnukItem){
                .type = SNUK_ITEM_RETURN,
                .expr = value,
            };
            break;
        case SNUK_TOKEN_BREAK:
            *item = (SnukItem){
                .type = SNUK_ITEM_BREAK,
                .expr = value,
            };
            break;
        case SNUK_TOKEN_CONTINUE:
            *item = (SnukItem){
                .type = SNUK_ITEM_CONTINUE,
            };
            break;
        default:
            SNUK_SHOULD_NOT_REACH_HERE;
            break;
    }
    return item;
}

/**
 * @brief Build a function declaration item.
 *
 * @param parser Parser context to operate on.
 * @param identifier Function name expression.
 * @param params Dynamic array of parameter nodes.
 * @param body Function body block.
 *
 * @return Newly allocated function item.
 */
SNUK_INLINE SnukItem *build_fn_item(SnukParser *parser, SnukExpr *identifier, SnukParam **params, SnukExpr *body, SnukExpr *return_type) {
    SnukItem *item = parser_create_item(parser);
    *item = (SnukItem){
        .type = SNUK_ITEM_FN_DECL,
        .fn_decl = {.identifier = identifier, .params = params, .body = body, .return_type = return_type},
    };
    return item;
}

/**
 * @brief Build a type declaration item.
 *
 * @param parser Parser context to operate on.
 * @param identifier Type name expression.
 * @param vars Dynamic array of field declarations.
 * @param fns Dynamic array of method declarations.
 *
 * @return Newly allocated type item.
 */
SNUK_INLINE SnukItem *build_type_item(SnukParser *parser, SnukExpr *identifier, SnukItem **vars, SnukItem **fns) {
    SnukItem *item = parser_create_item(parser);
    *item = (SnukItem){
        .type = SNUK_ITEM_TYPE_DECL,
        .type_decl = {.identifier = identifier, .vars = vars, .fns = fns},
    };
    return item;
}

/**
 * @brief Build or append to a print item.
 *
 * @param parser Parser context to operate on.
 * @param item Existing print item to append to, or NULL to create one.
 * @param expr Expression to append.
 *
 * @return Print item.
 */
SNUK_INLINE SnukItem *build_print_item(SnukParser *parser, SnukItem *item, SnukExpr *expr) {
    if (!item) {
        item = parser_create_item(parser);
        *item = (SnukItem){
            .type = SNUK_ITEM_PRINT,
            .print_exprs = snuk_darray_create(SnukExpr *),
        };
    }
    if (expr) snuk_darray_push(&item->print_exprs, expr);
    return item;
}

/**
 * @brief Build a comment item from a comment token.
 *
 * @param parser Parser context to operate on.
 * @param comment_token Source comment token.
 *
 * @return Newly allocated comment item.
 */
SNUK_INLINE SnukItem *build_comment_item(SnukParser *parser, SnukToken comment_token) {
    SnukItem *item = parser_create_item(parser);
    *item = (SnukItem){
        .type = comment_token.type == SNUK_TOKEN_BLOCK_COMMENT ? SNUK_ITEM_LINE_COMMENT : SNUK_ITEM_BLOCK_COMMENT,
        .comment = comment_token.string_literal,
    };
    return item;
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
 * @brief Build an if expression node.
 *
 * @param parser Parser context to operate on.
 * @param condition Condition expression to check.
 * @param then_block Block expression to execute on true condition.
 * @param else_block Block expression to execute on false condition.
 *
 * @return Newly allocated if expression node.
 */
SNUK_INLINE SnukExpr *build_if_expr(SnukParser *parser, SnukExpr *condition, SnukExpr *then_block, SnukExpr *else_block) {
    SnukExpr *expr = parser_create_expr(parser);
    *expr = (SnukExpr){
        .type = SNUK_EXPR_IF,
        .if_else = {.condition = condition, .then_block = then_block, .else_block = else_block},
    };
    return expr;
}

/**
 * @brief Build an match expression node.
 *
 * @param parser Parser context to operate on.
 * @param value Value expression to match.
 *
 * @return Newly allocated match expression node.
 */
SNUK_INLINE SnukExpr *build_match_expr(SnukParser *parser, SnukExpr *value) {
    SnukExpr *expr = parser_create_expr(parser);
    *expr = (SnukExpr){
        .type = SNUK_EXPR_MATCH,
        .match = {.value = value},
    };
    return expr;
}

/**
 * @brief Build an while or do while expression node.
 *
 * @param parser Parser context to operate on.
 * @param condition Condition expression to check.
 * @param body Block expression to execute.
 *
 * @return Newly allocated while or do while expression node.
 */
SNUK_INLINE SnukExpr *build_while_expr(SnukParser *parser, SnukExpr *condition, SnukExpr *body, bool is_do_while) {
    SnukExpr *expr = parser_create_expr(parser);
    *expr = (SnukExpr){
        .type = is_do_while ? SNUK_EXPR_DO_WHILE : SNUK_EXPR_WHILE,
        .while_loop = {.condition = condition, .body = body},
    };
    return expr;
}

/**
 * @brief Build an for expression node.
 *
 * @param parser Parser context to operate on.
 * @param init Initializer item.
 * @param condition Condition expression to check.
 * @param update Update expression.
 * @param body Block expression to execute.
 *
 * @return Newly allocated for expression node.
 */
SNUK_INLINE SnukExpr *build_for_expr(SnukParser *parser, SnukItem *init, SnukExpr *condition, SnukExpr *update, SnukExpr *body) {
    SnukExpr *expr = parser_create_expr(parser);
    *expr = (SnukExpr){
        .type = SNUK_EXPR_FOR,
        .for_loop = {.init = init, .condition = condition, .update = update, .body = body},
    };
    return expr;
}

/**
 * @brief Build an fn expression node.
 *
 * @param parser Parser context to operate on.
 * @param params Parameters of the fn expression.
 * @param body Block expression to execute.
 * @param return_type Return type of the function.
 *
 * @return Newly allocated fn expression node.
 */
SNUK_INLINE SnukExpr *build_fn_expr(SnukParser *parser, SnukParam **params, SnukExpr *body, SnukExpr *return_type) {
    SnukExpr *expr = parser_create_expr(parser);
    *expr = (SnukExpr){
        .type = SNUK_EXPR_FN,
        .fn_expr = {.params = params, .body = body, .return_type = return_type},
    };
    return expr;
}

/**
 * @brief Build an type expression node.
 *
 * @param parser Parser context to operate on.
 * @param vars Declaraiton items inside type.
 * @param fns Declaration items inside type.
 *
 * @return Newly allocated type expression node.
 */
SNUK_INLINE SnukExpr *build_type_expr(SnukParser *parser, SnukItem **vars, SnukItem **fns) {
    SnukExpr *expr = parser_create_expr(parser);
    *expr = (SnukExpr){
        .type = SNUK_EXPR_TYPE,
        .type_expr = {.vars = vars, .fns = fns},
    };
    return expr;
}

/**
 * @brief Build an block expression node.
 *
 * @param parser Parser context to operate on.
 * @param expr Existing block expression to append to, or NULL to create one.
 * @param item The item to append.
 *
 * @return Newly allocated block expression node.
 */
SNUK_INLINE SnukExpr *build_block_expr(SnukParser *parser, SnukExpr *expr, SnukItem *item) {
    if (!expr) {
        expr = parser_create_expr(parser);
        *expr = (SnukExpr){
            .type = SNUK_EXPR_BLOCK,
            .block_items = snuk_darray_create(SnukItem *),
        };
    }
    if (item) snuk_darray_push(&expr->block_items, item);
    return expr;
}

/**
 * @brief Build an call expression node.
 *
 * @param parser Parser context to operate on.
 *
 * @return Newly allocated call expression node.
 */
SNUK_INLINE SnukExpr *build_call_expr(SnukParser *parser) {
    SNUK_UNUSED(parser);
    return NULL;
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
