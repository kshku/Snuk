#pragma once

#include "parser_common.h"
#include "snuk/darray.h"
#include "snuk/defines.h"
#include "snuk/string_view.h"

/**
 * @brief Parsed type.
 */
struct SnukType {
    enum {
        TYPE_ANY, /**< No type annotation */
        TYPE_TYPE, /**< Type type */
        TYPE_NAMED, /**< Named type */
        TYPE_FN, /**< Function type */
        TYPE_INTERFACE, /**< Interface type */

        TYPE_MAX, /**< Sentinel value for type kinds. */
    } type;

    union {
        SnukStringView name; /**< Type name */

        struct {
            SnukType **param_types; /**< Darray of parameter types */
            SnukType *return_type; /**< The return type of function */
        } fn;

        SnukVar **members; /**< Members of the inerface */
    };
};

extern SnukType any_type;
extern SnukType type_type;

bool snuk_type_equal(SnukType *type1, SnukType *type2);

/**
 * @brief Allocate a type node.
 *
 * @param parser Parser context to operate on.
 *
 * @return Newly allocated expression storage.
 */
SNUK_INLINE SnukType *parser_create_type(SnukParser *parser) {
    SnukType *type = (SnukType *)parser->allocator->alloc(
        parser->allocator->data, sizeof(SnukType), alignof(SnukType));
    SNUK_ASSERT(type, "allocator is full, increase memory size!");
    return type;
}

/**
 * @brief Build a any type.
 *
 * @param parser Parser context to operate on.
 *
 * @return Newly allocated type node.
 */
SNUK_INLINE SnukType *build_any_type(SnukParser *parser) {
    SNUK_UNUSED(parser);
    return &any_type;
}

/**
 * @brief Build a type type.
 *
 * @param parser Parser context to operate on.
 *
 * @return Newly allocated type node.
 */
SNUK_INLINE SnukType *build_type_type(SnukParser *parser) {
    SNUK_UNUSED(parser);
    return &type_type;
}

/**
 * @brief Build a named type.
 *
 * @param parser Parser context to operate on.
 * @param name The type name.
 *
 * @return Newly allocated type node.
 */
SNUK_INLINE SnukType *build_named_type(SnukParser *parser, SnukStringView name) {
    SnukType *type = parser_create_type(parser);
    *type = (SnukType){
        .type = TYPE_NAMED,
        .name = parser_copy_string_view(parser, name),
    };
    return type;
}

/**
 * @brief Build a fn type.
 *
 * @param parser Parser context to operate on.
 * @param type Existing fn type to append param type or NULL to create one.
 * @param param The parameter type to append.
 * @param ret The return type of function.
 *
 * @return Newly allocated type node.
 *
 * @note param and ret will be only used if they are not NULL.
 */
SNUK_INLINE SnukType *build_fn_type(SnukParser *parser, SnukType *type, SnukType *param, SnukType *ret) {
    if (!type) {
        type = parser_create_type(parser);
        *type = (SnukType){
            .type = TYPE_FN,
            .fn = {.param_types = snuk_darray_create(SnukType *, parser->allocator)},
        };
    }
    if (param) snuk_darray_push(&type->fn.param_types, param);
    if (ret) type->fn.return_type = ret;
    return type;
}

SNUK_INLINE SnukType *build_interface_type(SnukParser *parser, SnukType *type, SnukVar *member) {
    if (!type) {
        type = parser_create_type(parser);
        *type = (SnukType){
            .type = TYPE_INTERFACE,
            .members = snuk_darray_create(SnukVar *, parser->allocator),
        };
    }

    if (member) snuk_darray_push(&type->members, member);

    return type;
}

/**
 * @breif Parse a type annotation.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed type, or NULL on parse failure.
 */
SnukType *snuk_type_parse(SnukParser *parser);

SnukType *snuk_type_parse_interface(SnukParser *parser);

/**
 * @brief Log a parsed type annotation.
 *
 * @param type The type to log.
 */
void snuk_type_log(SnukType *type);

SnukType *snuk_type_copy(SnukType *type);

void snuk_type_free(SnukType *type);
