#pragma once

#include "defines.h"
#include "refcount.h"
#include "snuk_env.h"

#define GET_SCOPE(rc) ((SnukScope *)snuk_ref_counter_get(rc))

typedef struct SnukScope SnukScope;

/**
 * @brief Lexical scope holding variable bindings and a parent reference.
 *
 * vars is a darray of owned SnukEnv pointers. parent is a refcounted handle
 * to the enclosing scope, or NULL for the global scope.
 */
struct SnukScope {
    SnukEnv **vars;  // darray
    SnukRefCounter *parent;
};

SNUK_INLINE void snuk_scope_destroy_envs(SnukScope *scope) {
    uint64_t count = snuk_darray_get_length(scope->vars);
    for (uint64_t i = 0; i < count; ++i) {
        SnukEnv *env;
        snuk_darray_pop(&scope->vars, &env);
        snuk_env_free(env);
    }
}

/**
 * @brief Refcount finalizer for a SnukScope.
 *
 * Destroys the bindings darray, releases the parent reference, and frees the
 * scope allocation. Used as the destructor callback when wrapping a scope
 * with snuk_ref_counter_create.
 *
 * @param data Unused user data pointer required by the refcount finalizer
 * signature.
 * @param ptr Pointer to the SnukScope being released.
 */
SNUK_INLINE void snuk_scope_destroy(void *data, void *ptr) {
    SNUK_UNUSED(data);
    SnukScope *scope = (SnukScope *)ptr;

    snuk_scope_destroy_envs(scope);

    snuk_darray_destroy(scope->vars);

    if (scope->parent) snuk_ref_counter_release(&scope->parent);

    snuk_free(scope);
}

/**
 * @brief Allocate a new scope and wrap it in a refcounter.
 *
 * @param parent Parent scope reference, consumed by this call. Pass NULL to
 * create a root scope.
 *
 * @return Refcounted handle to the new scope, with snuk_scope_free as the
 * finalizer.
 */
SNUK_INLINE SnukRefCounter *snuk_scope_create(SnukRefCounter *parent) {
    SnukScope *scope = (SnukScope *)snuk_alloc(sizeof(SnukScope), alignof(SnukScope));
    *scope = (SnukScope){
        .vars = snuk_darray_create(SnukEnv *, NULL),
        .parent = snuk_ref_counter_move(&parent),
    };
    return snuk_ref_counter_create(scope, NULL, snuk_scope_destroy);
}

/**
 * @brief Find a binding by name within a single scope, without walking parents.
 */
SNUK_INLINE SnukEnv *snuk_scope_lookup(SnukScope *scope, SnukStringView name) {
    uint64_t count = snuk_darray_get_length(scope->vars);
    for (uint64_t i = 0; i < count; ++i)
        if (snuk_string_view_equal(scope->vars[i]->name, name)) return scope->vars[i];

    return NULL;
}

/**
 * @brief Append a binding to a scope's variable list.
 * Takes ownership of env regardless of success or failure.
 */
SNUK_INLINE bool snuk_scope_add_env(SnukScope *scope, SnukEnv *env) {
    if (snuk_scope_lookup(scope, env->name)) {
        log_error("multiple declaration of '" SNUK_STRING_VIEW_FORMAT "'", SNUK_STRING_VIEW_ARG(env->name));
        snuk_env_free(env);
        return false;
    }
    snuk_darray_push(&scope->vars, env);
    return true;
}

SNUK_INLINE void snuk_scope_remove_env(SnukScope *scope, SnukStringView name) {
    uint64_t count = snuk_darray_get_length(scope->vars);
    for (uint64_t i = 0; i < count; ++i) {
        if (snuk_string_view_equal(scope->vars[i]->name, name)) {
            SnukEnv *env = NULL;
            snuk_darray_pop_at(&scope->vars, i, &env);
            snuk_env_free(env);
            return;
        }
    }
}

SNUK_INLINE void snuk_scope_free_fn_closures(SnukScope *scope) {
    uint64_t count = snuk_darray_get_length(scope->vars);
    for (uint64_t i = 0; i < count; ++i) {
        SnukEnv *env = scope->vars[i];
        if (env->value.type == SNUK_VALUE_FN && env->value.fn_value.closure)
            snuk_ref_counter_release(&env->value.fn_value.closure);
    }
}
