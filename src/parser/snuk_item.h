#pragma once

#include "darray.h"
#include "defines.h"
#include "parser_common.h"
#include "string_view.h"

/**
 * @brief Parser items kinds.
 */
typedef enum SnukItemType {
    SNUK_ITEM_EXPR, /**< Expression items */

    SNUK_ITEM_VAR_DECL, /**< Variable declaration (expression with restriction)
                         */
    SNUK_ITEM_CONST_DECL, /**< Constant declaration (expression with
                             restriction) */

    SNUK_ITEM_PRINT, /**< Printing expression (special expression, always
                        returns null) */

    SNUK_ITEM_RETURN, /**< Return expression, transfer control out of function,
                         may carry value with them */
    SNUK_ITEM_BREAK, /**< Transfer control out of loop, may carray value with
                        them. */
    SNUK_ITEM_CONTINUE, /**< Transfer control to next iteration. */

    SNUK_ITEM_MAX /**< Sentinel value for item kinds. */
} SnukItemType;

/**
 * @brief Parsed item.
 */
struct SnukItem {
        SnukItemType
            type; /**< Discriminant selecting the active item payload. */

        union {
                SnukExpr *expr; /**< expression item payload (also used for
                                   return and break). */

                struct {
                        SnukStringView name; /**< Name. */
                        SnukType *type; /**< Type information. */
                        SnukExpr *expr; /**< Expression. */
                } decl_item;

                SnukExpr *
                    *print_exprs; /**< Dynamic array of expressions to print. */
        };
};

/**
 * @brief Allocate a item.
 *
 * @param parser Parser context to operate on.
 *
 * @return Newly allocated item storage.
 */
SNUK_INLINE SnukItem *parser_create_item(SnukParser *parser) {
    return (SnukItem *)parser->allocator->alloc(
        parser->allocator->data, sizeof(SnukItem), alignof(SnukItem));
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
 * @param name Declared name expression.
 * @param type Type of the variable or constant.
 * @param expr Expression value.
 * @param item_type Type of the item.
 *
 * @return Newly allocated declaration item.
 */
SNUK_INLINE SnukItem *build_decl_item(
    SnukParser *parser, SnukStringView name, SnukType *type, SnukExpr *expr,
    SnukItemType item_type) {
    SnukItem *item = parser_create_item(parser);
    *item = (SnukItem){
        .type = item_type,
        .decl_item =
            {.name = parser_copy_string_view(parser, name),
                        .type = type,
                        .expr = expr},
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
SNUK_INLINE SnukItem *build_flow_item(
    SnukParser *parser, SnukTokenType type, SnukExpr *value) {
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
 * @brief Build or append to a print item.
 *
 * @param parser Parser context to operate on.
 * @param item Existing print item to append to, or NULL to create one.
 * @param expr Expression to append.
 *
 * @return Print item.
 */
SNUK_INLINE SnukItem *build_print_item(
    SnukParser *parser, SnukItem *item, SnukExpr *expr) {
    if (!item) {
        item = parser_create_item(parser);
        *item = (SnukItem){
            .type = SNUK_ITEM_PRINT,
            .print_exprs = snuk_darray_create(SnukExpr *, parser->allocator),
        };
    }
    if (expr) snuk_darray_push(&item->print_exprs, expr);
    return item;
}

/**
 * @brief Parse the next item.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed item, or NULL on parse failure.
 */
SnukItem *snuk_item_parse(SnukParser *parser);

/**
 * @brief Get a string name for a item type.
 *
 * @param type Item type to convert.
 *
 * @return Static string describing the item type.
 */
const char *snuk_item_type_to_string(SnukItemType type);

/**
 * @brief Log a parsed item tree.
 *
 * @param item item to log.
 */
void snuk_item_log(SnukItem *item);

