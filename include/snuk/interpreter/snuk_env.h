#pragma once

#include "snuk/darray.h"
#include "snuk/defines.h"
#include "snuk/parser/snuk_type.h"
#include "snuk/string_view.h"
#include "snuk_value.h"

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
SNUK_INLINE SnukEnv *snuk_env_create(SnukStringView name, SnukType *type, SnukValue value) {
    SnukEnv *env = (SnukEnv *)snuk_alloc(sizeof(SnukEnv), alignof(SnukEnv));
    *env = (SnukEnv){
        .name = name,
        .type = type,
        .value = snuk_value_copy(value),
    };
    return env;
}

SNUK_INLINE void snuk_env_assign_value(SnukEnv *env, SnukValue value) {
    snuk_value_free(env->value);
    env->value = snuk_value_copy(value);
}

SNUK_INLINE void snuk_env_free(SnukEnv *env) {
    if (!env) return;

    snuk_value_free(env->value);

    snuk_free(env);
}
