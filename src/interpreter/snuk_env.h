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
SNUK_INLINE SnukEnv *snuk_create_env(SnukStringView name, SnukValue value) {
    SnukEnv *env = (SnukEnv *)snuk_alloc(sizeof(SnukEnv), alignof(SnukEnv));
    *env = (SnukEnv){
        .name = snuk_string_view_copy(name),
        .value = snuk_interpreter_copy_value(value),
    };
    return env;
}

SNUK_INLINE void snuk_free_env(SnukEnv *env) {
    if (!env) return;

    snuk_free((void *)env->name.str);
    snuk_interpreter_free_value(env->value);

    snuk_free(env);
}
