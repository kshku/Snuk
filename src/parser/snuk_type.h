#pragma once

#include "darray.h"
#include "defines.h"
#include "parser_common.h"
#include "string_view.h"

/**
 * @brief Parsed type.
 */
struct SnukType {
        enum {
            TYPE_ANY, /**< No type annotation */
            TYPE_NAMED, /**< Named type */
            TYPE_FN, /**< Function type */
            TYPE_TYPE, /**< Type type */

            TYPE_MAX /**< Sentinel value for type kinds. */
        } type;

        union {
                SnukStringView name; /**< Type name (for named parameters) */

                struct {
                        SnukType
                            *return_type; /**< The return type of function */
                        SnukType *
                            *param_types; /**< Darray of parameter types */
                } fn;

                SnukType **member_types; /**< Darray of type of the members */
        };
};

/**
 * @brief Allocate a type node.
 *
 * @param parser Parser context to operate on.
 *
 * @return Newly allocated expression storage.
 */
SNUK_INLINE SnukType *parser_create_type(SnukParser *parser) {
    return (SnukType *)parser->allocator->alloc(
        parser->allocator->data, sizeof(SnukType), alignof(SnukType));
}

/**
 * @brief Build a any type.
 *
 * @param parser Parser context to operate on.
 *
 * @return Newly allocated type node.
 */
SNUK_INLINE SnukType *build_any_type(SnukParser *parser) {
    SnukType *type = parser_create_type(parser);
    *type = (SnukType){
        .type = TYPE_ANY,
    };
    return type;
}

/**
 * @brief Build a named type.
 *
 * @param parser Parser context to operate on.
 * @param name The type name.
 *
 * @return Newly allocated type node.
 */
SNUK_INLINE SnukType *build_named_type(
    SnukParser *parser, SnukStringView name) {
    SnukType *type = parser_create_type(parser);
    *type = (SnukType){
        .type = TYPE_NAMED,
        .name = name,
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
SNUK_INLINE SnukType *build_fn_type(
    SnukParser *parser, SnukType *type, SnukType *param, SnukType *ret) {
    if (!type) {
        type = parser_create_type(parser);
        *type = (SnukType){
            .type = TYPE_FN,
            .fn =
                {.param_types =
                     snuk_darray_create(SnukType *, parser->allocator)},
        };
    }
    if (param) snuk_darray_push(&type->fn.param_types, param);
    if (ret) type->fn.return_type = ret;
    return type;
}

/**
 * @brief Build a type type.
 *
 * @param parser Parser context to operate on.
 * @param type Existing type type to append member type or NULL to create one.
 * @param member_type The member type to append.
 *
 * @return Newly allocated type node.
 */
SNUK_INLINE SnukType *build_type_type(
    SnukParser *parser, SnukType *type, SnukType *member_type) {
    if (!type) {
        type = parser_create_type(parser);
        *type = (SnukType){
            .type = TYPE_TYPE,
            .member_types = snuk_darray_create(SnukType *, parser->allocator),
        };
    }
    if (member_type) snuk_darray_push(&type->member_types, member_type);
    return type;
}

/**
 * @breif Parse a type annotation.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed type, or NULL on parse failure.
 */
SnukType *parse_type_annot(SnukParser *parser, ParseFlag flag);

/**
 * @brief Log a parsed type annotation.
 *
 * @param type The type to log.
 */
void snuk_parser_log_type(SnukType *type);
