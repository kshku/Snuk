#pragma once

#include "defines.h"
#include "snuk_value.h"
#include "string_view.h"

typedef struct SnukEnv SnukEnv;

/**
 * @brief Single name-to-value binding inside a scope.
 */
struct SnukEnv {
        SnukStringView name;
        SnukValue value;
};

/**
 * @brief Allocate a SnukEnv and populate it by evaluating the given expression.
 */
SNUK_INLINE SnukEnv *snuk_env_create(SnukStringView name, SnukValue value) {
    SnukEnv *env = (SnukEnv *)snuk_alloc(sizeof(SnukEnv), alignof(SnukEnv));
    *env = (SnukEnv){
        .name = snuk_string_view_copy(name),
        .value = snuk_value_copy(value),
    };
    return env;
}

SNUK_INLINE void snuk_env_free(SnukEnv *env) {
    if (!env) return;

    snuk_free((void *)env->name.str);
    snuk_value_free(env->value);

    snuk_free(env);
}
