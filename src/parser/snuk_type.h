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
        TYPE_INTERFACE,

        TYPE_MAX, /**< Sentinel value for type kinds. */
    } type;

    union {
        SnukStringView name; /**< Type name (for named parameters) */

        struct {
            SnukType *return_type; /**< The return type of function */
            SnukType **param_types; /**< Darray of parameter types */
        } fn;

        SnukType **member_types; /**< Darray of type of the members */
    };
};

SNUK_INLINE bool snuk_type_equal(SnukType *type1, SnukType *type2) {
    if (type1->type != type2->type) return false;

    uint64_t count1;
    uint64_t count2;
    switch (type1->type) {
        case TYPE_ANY:
            return true;

        case TYPE_NAMED:
            return snuk_string_view_equal(type1->name, type2->name);

        case TYPE_FN:
            if (!snuk_type_equal(type1->fn.return_type, type2->fn.return_type)) return false;

            count1 = snuk_darray_get_length(type1->fn.param_types);
            count2 = snuk_darray_get_length(type2->fn.param_types);
            if (count1 != count2) return false;

            for (uint64_t i = 0; i < count1; ++i)
                if (!snuk_type_equal(type1->fn.param_types[i], type2->fn.param_types[i]))
                    return false;
            return true;

        case TYPE_TYPE:
            count1 = snuk_darray_get_length(type1->member_types);
            count2 = snuk_darray_get_length(type2->member_types);
            if (count1 != count2) return false;
            for (uint64_t i = 0; i < count1; ++i)
                if (!snuk_type_equal(type1->member_types[i], type2->member_types[i])) return false;

            return true;

        default:
            SNUK_SHOULD_NOT_REACH_HERE;
            break;
    }
    return false;
}

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

/**
 * @brief Build a type type.
 *
 * @param parser Parser context to operate on.
 * @param type Existing type type to append member type or NULL to create one.
 * @param member_type The member type to append.
 *
 * @return Newly allocated type node.
 */
SNUK_INLINE SnukType *build_type_type(SnukParser *parser, SnukType *type, SnukType *member_type) {
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

SNUK_INLINE SnukType *build_interface_type(SnukParser *parser, SnukStringView name) {
    SnukType *type = parser_create_type(parser);
    *type = (SnukType){
        .type = TYPE_INTERFACE,
        .name = parser_copy_string_view(parser, name),
    };
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

/**
 * @brief Log a parsed type annotation.
 *
 * @param type The type to log.
 */
void snuk_type_log(SnukType *type);

SnukType *snuk_type_copy(SnukType *type);

void snuk_type_free(SnukType *type);
