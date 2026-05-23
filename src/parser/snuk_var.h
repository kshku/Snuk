#pragma once

#include "defines.h"
#include "parser_common.h"
#include "string_view.h"

typedef struct SnukVar {
    SnukStringView name; /**< Name of the variable. */
    SnukType *type; /**< Type information. */
    SnukExpr *value; /**< Value of variable. */
} SnukVar;

SNUK_INLINE SnukVar *parser_create_var(SnukParser *parser) {
    SnukVar *var
        = (SnukVar *)parser->allocator->alloc(parser->allocator->data, sizeof(SnukVar), alignof(SnukVar));
    SNUK_ASSERT(var, "allocator is full, increase memory size!");
    return var;
}

/**
 * @breif Build a var node.
 *
 * @param parser Parser context to operate on.
 * @param name Name of the variable.
 * @param type Type information.
 * @param value Value of the variable.
 *
 * @return Newly allocated var node.
 */
SNUK_INLINE SnukVar *build_var(SnukParser *parser, SnukStringView name, SnukType *type, SnukExpr *value) {
    SnukVar *var = parser_create_var(parser);
    *var = (SnukVar){
        .name = parser_copy_string_view(parser, name),
        .type = type,
        .value = value,
    };
    return var;
}

void snuk_var_log(SnukVar *var);

SnukVar *snuk_var_parse(SnukParser *parser, bool default_null);
