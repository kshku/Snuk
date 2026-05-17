#pragma once

#include "defines.h"
#include "parser_common.h"
#include "string_view.h"

/**
 * @brief Parsed function parameter.
 */
struct SnukParam {
        SnukStringView name; /**< Parameter name expression. */
        SnukType *type; /**< Type information of the parameter */
        SnukExpr *default_value; /**< Optional default value expression. */
};

/**
 * @brief Allocate a parameter node.
 *
 * @param parser Parser context to operate on.
 *
 * @return Newly allocated parameter storage.
 */
SNUK_INLINE SnukParam *parser_create_param(SnukParser *parser) {
    return (SnukParam *)parser->allocator->alloc(
        parser->allocator->data, sizeof(SnukParam), alignof(SnukParam));
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
SNUK_INLINE SnukParam *build_param(
    SnukParser *parser, SnukStringView name, SnukType *type,
    SnukExpr *default_value) {
    SnukParam *param = parser_create_param(parser);
    *param = (SnukParam){
        .name = name,
        .type = type,
        .default_value = default_value,
    };
    return param;
}

/**
 * @brief Log a parsed function parameter.
 *
 * @param param Parameter to log.
 */
void snuk_parser_log_param(SnukParam *param);
