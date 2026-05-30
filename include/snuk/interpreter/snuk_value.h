#pragma once

#include "snuk/defines.h"
#include "snuk/parser/snuk_expr.h"
#include "snuk/parser/snuk_type.h"
#include "snuk/parser/snuk_var.h"
#include "snuk/refcount.h"
#include "snuk/string_view.h"

typedef struct SnukValue SnukValue;
typedef struct SnukInterpreter SnukInterpreter;

typedef SnukValue (*builtin_function)(SnukInterpreter *intpret);

typedef enum SnukValueType {
    SNUK_VALUE_UNKOWN,
    SNUK_VALUE_INT,
    SNUK_VALUE_FLOAT,
    SNUK_VALUE_BOOL,
    SNUK_VALUE_STRING,
    SNUK_VALUE_NULL,
    SNUK_VALUE_FN,
    SNUK_VALUE_FN_BUILTIN,
    SNUK_VALUE_TYPE,
    SNUK_VALUE_TYPE_INST,
    SNUK_VALUE_INTERFACE,
    SNUK_VALUE_ERROR,

    SNUK_VALUE_MAX,
} SnukValueType;

/**
 * @brief Runtime value produced by expression evaluation.
 *
 * The type tag selects which union member is meaningful: int_value for
 * integers, float_value for floats, bool_value for booleans, string_value for
 * string views, and fn_value for function values. SNUK_VALUE_NULL,
 * SNUK_VALUE_UNKOWN, and SNUK_VALUE_TYPE carry no payload today.
 *
 * @note A function value holds a refcounted reference to its closure scope,
 * which keeps the captured bindings alive for as long as the function value
 * is reachable.
 */
struct SnukValue {
    SnukValueType type;

    union {
        int64_t int_value;
        double float_value;
        bool bool_value;
        SnukStringView string_value;

        struct {
            SnukRefCounter *instance;
            SnukRefCounter *closure;
            bool weak_ref;
            SnukExpr *body;
            SnukType *type;
        } fn_value;

        struct {
            SnukRefCounter *instance;
            SnukRefCounter *closure;
            builtin_function fn;
            SnukType *type;
        } builtin_fn;

        struct {
            SnukRefCounter *type_scope;
            SnukRefCounter *closure;
            bool weak_ref;
            SnukType *type;
        } type_value;

        struct {
            SnukType *type;
        } interface;

        struct {
            const char *msg;
            SnukToken token;
        } error;
    };
};

/**
 * @brief Coerce a runtime value to a boolean for conditions and loops.
 */
SNUK_INLINE bool snuk_value_is_true(SnukValue value) {
    switch (value.type) {
        case SNUK_VALUE_UNKOWN:
        case SNUK_VALUE_NULL:
            return false;
        case SNUK_VALUE_BOOL:
            return value.bool_value;
        case SNUK_VALUE_INT:
            return value.int_value != 0;
        case SNUK_VALUE_FLOAT:
            return value.float_value != 0;
        case SNUK_VALUE_STRING:
            return value.string_value.len != 0;

        case SNUK_VALUE_FN:
        case SNUK_VALUE_TYPE:
            return true;

        case SNUK_VALUE_TYPE_INST:
            return value.type_value.closure != NULL;

        default:
            return false;
    }
}

/**
 * @brief Copy the value.
 *
 * @param value Value to copy.
 */
SnukValue snuk_value_copy(SnukValue value);

/**
 * @brief Free the value.
 *
 * Releases the resources held by value.
 *
 * @param value Value to free.
 */
void snuk_value_free(SnukValue value);

/**
 * @brief Log a runtime value to the trace logger for debugging.
 *
 * @param value Value to log.
 */
void snuk_value_log(SnukValue value);

/**
 * @brief Print a runtime value to standard output.
 *
 * @param value Value to print.
 */
void snuk_value_print(SnukValue value);

