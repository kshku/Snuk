#pragma once

#include "darray.h"
#include "defines.h"
#include "parser/snuk_type.h"
#include "snuk_value.h"
#include "string_view.h"

typedef struct SnukEnv SnukEnv;

/**
 * @brief Single name-to-value binding inside a scope.
 */
struct SnukEnv {
        SnukStringView name;
        SnukType *type;
        SnukValue value;
};

/**
 * @brief Allocate a SnukEnv and populate it by evaluating the given expression.
 */
SNUK_INLINE SnukEnv *snuk_env_create(
    SnukStringView name, SnukType *type, SnukValue value) {
    SnukEnv *env = (SnukEnv *)snuk_alloc(sizeof(SnukEnv), alignof(SnukEnv));
    *env = (SnukEnv){
        .name = name,
        .type = type,
        .value = snuk_value_copy(value),
    };
    return env;
}

SNUK_INLINE void snuk_env_free(SnukEnv *env) {
    if (!env) return;

    snuk_value_free(env->value);

    snuk_free(env);
}
