#pragma once

#include "darray.h"
#include "defines.h"
#include "parser_common.h"
#include "string_view.h"

/**
 * @brief Parser expression node kinds.
 */
typedef enum SnukExprType {
    SNUK_EXPR_IDENTIFIER, /**< Identifier reference expression. */
    SNUK_EXPR_INT, /**< Integer literal expression. */
    SNUK_EXPR_FLOAT, /**< Floating-point literal expression. */
    SNUK_EXPR_STRING, /**< String literal expression. */
    SNUK_EXPR_BOOL, /**< Boolean literal expression. */
    SNUK_EXPR_NULL, /**< Null literal expression. */

    SNUK_EXPR_UNARY, /**< Unary operator expression. */
    SNUK_EXPR_BINARY, /**< Binary operator expression. */

    SNUK_EXPR_ASSIGN, /**< Assignment expression. */
    SNUK_EXPR_COMPOUND_ASSIGN, /**< Compound assignment expression. */

    SNUK_EXPR_IF, /**< If expression. */
    SNUK_EXPR_MATCH, /**< Match expression. */

    SNUK_EXPR_WHILE, /**< while loop expression. */
    SNUK_EXPR_DO_WHILE, /**< do-while loop expression. */
    SNUK_EXPR_FOR, /**< for loop expression. */

    SNUK_EXPR_FN, /**< Funtion expression */
    SNUK_EXPR_TYPE, /**< Type expression */
    SNUK_EXPR_TYPE_INST, /**< Type instance */

    SNUK_EXPR_BLOCK, /**< Block expression */

    SNUK_EXPR_CALL, /**< Function call expression. */
    SNUK_EXPR_MEMBER, /**< Member access expression. */
    SNUK_EXPR_INDEX, /**< Index access expression. */

    SNUK_EXPR_LINE_COMMENT, /**< Single line comment */
    SNUK_EXPR_BLOCK_COMMENT, /**< Multi line comment */

    SNUK_EXPR_MAX /**< Sentinel value for expression kinds. */
} SnukExprType;

/**
 * @brief Parsed expression node.
 */
struct SnukExpr {
        SnukExprType
            type; /**< Discriminant selecting the active expression payload. */

        union {
                SnukStringView identifier;
                int64_t int_literal;
                double float_literal;
                SnukStringView string_literal;
                bool bool_literal;
                SnukStringView comment; /**< Comment */

                struct {
                        SnukTokenType op; /**< Unary operator token. */
                        SnukExpr *operand; /**< Unary operand expression. */
                } unary;

                struct {
                        SnukTokenType op; /**< Binary operator token. */
                        SnukExpr *left; /**< Left-hand operand expression. */
                        SnukExpr *right; /**< Right-hand operand expression. */
                } binary;

                struct {
                        SnukExpr *identifier; /**< Assignment target identifier
                                                 expression. */
                        SnukExpr *value; /**< Assigned value expression. */
                } assign;

                struct {
                        SnukTokenType op; /**< Compound assignment token */
                        SnukExpr *identifier; /**< Assignment target identifier
                                                 expression. */
                        SnukExpr *value; /**< Assigned value expression. */
                } compound_assign;

                struct {
                        SnukExpr *condition; /**< Condition expression. */
                        SnukExpr *then_block; /**< Block expression to execute
                                                 on true condition */
                        SnukExpr *else_block; /**< Block expression to execute
                                                 on false condition */
                } if_else;

                struct {
                        SnukExpr *value; /**< Value expression being matched. */
                        // TODO:
                } match;

                struct {
                        SnukExpr *condition; /**< Loop condition expression. */
                        SnukExpr *body; /**< Loop body block. */
                } while_loop;  // while, do while

                struct {
                        SnukItem *init; /**< Optional initializer. */
                        SnukExpr *condition; /**< Optional loop condition
                                                expression. */
                        SnukExpr
                            *update; /**< Optional loop update expression. */
                        SnukExpr *body; /**< Loop body block. */
                } for_loop;

                struct {
                        SnukParam **params; /**< Darray of parameters. */
                        SnukExpr *body; /**< Body of function */
                        SnukStringView
                            name; /**< Name in case of syntax sugar */
                        SnukType *type; /**< Type of the function */
                } fn_expr;

                struct {
                        SnukItem **members; /**< Dynamic array of members items
                                               in the type */
                        SnukStringView
                            name; /**< Name in case of syntax sugar */
                        SnukType *type; /**< Type of the type */
                } type_expr;

                struct {
                        SnukType *type; /**< Name of the type */
                        SnukStringView
                            name; /**< Name in case of syntax sugar. */
                        SnukExpr **init; /**< initial values of members */
                } type_inst_expr;

                SnukItem *
                    *block_items; /**< Dynamic array of items in the block. */

                struct {
                        SnukExpr *fn; /**< Expression to call */
                        SnukExpr **
                            params; /**< Darray of call argument expressions. */
                } call;

                struct {
                        SnukExpr *type; /**< Type from which to access the
                                           field/member */
                        SnukExpr *field; /**< The field/member */
                } member_access;
        };
};

/**
 * @brief Allocate an expression node.
 *
 * @param parser Parser context to operate on.
 *
 * @return Newly allocated expression storage.
 */
SNUK_INLINE SnukExpr *parser_create_expr(SnukParser *parser) {
    return (SnukExpr *)parser->allocator->alloc(
        parser->allocator->data, sizeof(SnukExpr), alignof(SnukExpr));
}

/**
 * @brief Build a comment expresson from a comment token.
 *
 * @param parser Parser context to operate on.
 *
 * @return Newly allocated comment expressoin.
 */
SNUK_INLINE SnukExpr *build_comment_expr(
    SnukParser *parser, SnukToken comment_token) {
    SnukExpr *expr = parser_create_expr(parser);
    *expr = (SnukExpr){
        .type = comment_token.type == SNUK_TOKEN_BLOCK_COMMENT
                  ? SNUK_EXPR_BLOCK_COMMENT
                  : SNUK_EXPR_LINE_COMMENT,
        .comment =
            parser_copy_string_view(parser, comment_token.string_literal),
    };
    return expr;
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
        .type = SNUK_EXPR_NULL,
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
        .type = SNUK_EXPR_BOOL,
        .bool_literal = parser->previous.type == SNUK_TOKEN_TRUE,
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
        .type = SNUK_EXPR_STRING,
        .string_literal =
            parser_copy_string_view(parser, parser->previous.string_literal),
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
        .identifier =
            parser_copy_string_view(parser, parser->previous.string_literal),
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
        .type = SNUK_EXPR_INT,
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
        .type = SNUK_EXPR_FLOAT,
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
SNUK_INLINE SnukExpr *build_unary_expr(
    SnukParser *parser, SnukTokenType op, SnukExpr *operand) {
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
SNUK_INLINE SnukExpr *build_binary_expr(
    SnukParser *parser, SnukTokenType op, SnukExpr *left, SnukExpr *right) {
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
SNUK_INLINE SnukExpr *build_assign_expr(
    SnukParser *parser, SnukExpr *identifier, SnukExpr *value) {
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
SNUK_INLINE SnukExpr *build_compound_assign_expr(
    SnukParser *parser, SnukTokenType op, SnukExpr *identifier,
    SnukExpr *value) {
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
SNUK_INLINE SnukExpr *build_if_expr(
    SnukParser *parser, SnukExpr *condition, SnukExpr *then_block,
    SnukExpr *else_block) {
    SnukExpr *expr = parser_create_expr(parser);
    *expr = (SnukExpr){
        .type = SNUK_EXPR_IF,
        .if_else =
            {.condition = condition,
                      .then_block = then_block,
                      .else_block = else_block},
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
SNUK_INLINE SnukExpr *build_while_expr(
    SnukParser *parser, SnukExpr *condition, SnukExpr *body, bool is_do_while) {
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
SNUK_INLINE SnukExpr *build_for_expr(
    SnukParser *parser, SnukItem *init, SnukExpr *condition, SnukExpr *update,
    SnukExpr *body) {
    SnukExpr *expr = parser_create_expr(parser);
    *expr = (SnukExpr){
        .type = SNUK_EXPR_FOR,
        .for_loop =
            {.init = init,
                       .condition = condition,
                       .update = update,
                       .body = body},
    };
    return expr;
}

/**
 * @brief Build an fn expression node.
 *
 * @param parser Parser context to operate on.
 * @param params Parameters of the fn expression.
 * @param body Block expression to execute.
 * @param name Name of function in case of syntax sugar.
 * @param type The type of function.
 *
 * @return Newly allocated fn expression node.
 */
SNUK_INLINE SnukExpr *build_fn_expr(
    SnukParser *parser, SnukParam **params, SnukExpr *body, SnukStringView name,
    SnukType *type) {
    SnukExpr *expr = parser_create_expr(parser);
    *expr = (SnukExpr){
        .type = SNUK_EXPR_FN,
        .fn_expr =
            {.params = params,
                      .body = body,
                      .name = parser_copy_string_view(parser, name),
                      .type = type},
    };
    return expr;
}

/**
 * @brief Build an type expression node.
 *
 * @param parser Parser context to operate on.
 * @param members Member items in type.
 * @param name Name of type in case of syntax sugar.
 * @param type Type of the type
 *
 * @return Newly allocated type expression node.
 */
SNUK_INLINE SnukExpr *build_type_expr(
    SnukParser *parser, SnukItem **members, SnukStringView name,
    SnukType *type) {
    SnukExpr *expr = parser_create_expr(parser);
    *expr = (SnukExpr){
        .type = SNUK_EXPR_TYPE,
        .type_expr =
            {.members = members,
                        .name = parser_copy_string_view(parser, name),
                        .type = type},
    };
    return expr;
}

/**
 * @brief Build an type instance node.
 *
 * @param parser Parser context to operate on.
 * @param type The type of instance.
 * @param init Initialization values.
 * @param name Name in case of syntax sugar.
 *
 * @return Newly allocated type expression node.
 */
SNUK_INLINE SnukExpr *build_type_inst_expr(
    SnukParser *parser, SnukType *type, SnukExpr **init, SnukStringView name) {
    SnukExpr *expr = parser_create_expr(parser);
    *expr = (SnukExpr){
        .type = SNUK_EXPR_TYPE_INST,
        .type_inst_expr =
            {.type = type,
                             .init = init,
                             .name = parser_copy_string_view(parser, name)},
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
SNUK_INLINE SnukExpr *build_block_expr(
    SnukParser *parser, SnukExpr *expr, SnukItem *item) {
    if (!expr) {
        expr = parser_create_expr(parser);
        *expr = (SnukExpr){
            .type = SNUK_EXPR_BLOCK,
            .block_items = snuk_darray_create(SnukItem *, parser->allocator),
        };
    }
    if (item) snuk_darray_push(&expr->block_items, item);
    return expr;
}

/**
 * @brief Build a call expression node.
 *
 * @param parser Parser context to operate on.
 * @param fn Expression to call.
 * @param params The parameters.
 *
 * @return Newly allocated call expression node.
 */
SNUK_INLINE SnukExpr *build_call_expr(
    SnukParser *parser, SnukExpr *fn, SnukExpr **params) {
    SnukExpr *expr = parser_create_expr(parser);
    *expr = (SnukExpr){
        .type = SNUK_EXPR_CALL,
        .call = {.fn = fn, .params = params},
    };
    return expr;
}

/**
 * @brief Build member access expression node.
 *
 * @param parser Parser context to operate on.
 * @param type The type to access member from.
 * @param field The member to access.
 */
SNUK_INLINE SnukExpr *build_member_access_expr(
    SnukParser *parser, SnukExpr *type, SnukExpr *field) {
    SnukExpr *expr = parser_create_expr(parser);
    *expr = (SnukExpr){
        .type = SNUK_EXPR_MEMBER,
        .member_access = {.type = type, .field = field},
    };
    return expr;
}

/**
 * @brief Parse an expression from the lowest precedence.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed expression, or NULL on parse failure.
 */
SnukExpr *snuk_expr_parse(SnukParser *parser);

/**
 * @brief Get a string name for an expression type.
 *
 * @param type Expression type to convert.
 *
 * @return Static string describing the expression type.
 */
const char *snuk_expr_type_to_string(SnukExprType type);

/**
 * @brief Log a parsed expression tree.
 *
 * @param expr Expression to log.
 */
void snuk_expr_log(SnukExpr *expr);

