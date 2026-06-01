#pragma once

#include "interpreter.h"
#include "snuk/darray.h"
#include "snuk/defines.h"
#include "snuk_scope.h"

SNUK_INLINE SnukValue interpreter_error(SnukInterpreter *intpret, const char *err_msg) {
    if (intpret->panic_mode) return intpret->error;
    intpret->panic_mode = true;
    intpret->error = (SnukValue){
        .type = SNUK_VALUE_ERROR,
        .err_msg = err_msg,
    };
    return intpret->error;
}

#define SNUK_INTERPRETER_CHECK(intpret, cond, err_msg)       \
    if (!(cond)) return interpreter_error(intpret, err_msg);

/**
 * @brief Walk the scope chain from current to global to resolve a name.
 */
SNUK_INLINE SnukEnv *interpreter_lookup(SnukInterpreter *intpret, SnukStringView name) {
    SnukEnv *env = NULL;
    if (intpret->instance) {
        env = snuk_scope_lookup(intpret->current, name);
        if (!env) env = snuk_scope_lookup(intpret->instance, name);
    }
    if (!env) env = snuk_scope_lookup_recursive(intpret->current, name);
    return env;
}

/**
 * @brief Push a new child scope and make it the interpreter's current scope.
 */
SNUK_INLINE void interpreter_push_scope(SnukInterpreter *intpret) {
    intpret->current = snuk_scope_create(snuk_ref_counter_move(&intpret->current), false);
}

/**
 * @brief Pop the current scope and restore its parent as the active scope.
 */
SNUK_INLINE void interpreter_pop_scope(SnukInterpreter *intpret) {
    if (intpret->global == intpret->current) SNUK_SHOULD_NOT_REACH_HERE;

    SnukScope *scope = GET_SCOPE(intpret->current);
    SnukRefCounter *parent = snuk_ref_counter_retain(scope->parent);

    snuk_ref_counter_release(&intpret->current);

    intpret->current = snuk_ref_counter_move(&parent);
}

SNUK_INLINE SnukEnv *
    interpreter_get_member_env(SnukInterpreter *intpret, SnukValue type_or_inst, SnukStringView field) {
    SNUK_UNUSED(intpret);
    SNUK_ASSERT(type_or_inst.type == SNUK_VALUE_TYPE || type_or_inst.type == SNUK_VALUE_TYPE_INST, "something is wrong");

    // Do not lookup recursively
    SnukEnv *env = snuk_scope_lookup(type_or_inst.type_value.closure, field);
    if (!env && type_or_inst.type_value.type_scope)
        env = snuk_scope_lookup(type_or_inst.type_value.type_scope, field);
    return env;
}

SNUK_INLINE SnukValue interpreter_get_member(SnukInterpreter *intpret, SnukValue type_or_inst, SnukStringView field) {
    SnukEnv *env = interpreter_get_member_env(intpret, type_or_inst, field);
    if (!env) return (SnukValue){.type = SNUK_VALUE_UNKOWN};
    return snuk_value_copy(env->value);
}

SNUK_INLINE bool interpreter_set_member(
    SnukInterpreter *intpret, SnukValue type_or_inst, SnukStringView field, SnukValue value) {
    SNUK_ASSERT(type_or_inst.type == SNUK_VALUE_TYPE || type_or_inst.type == SNUK_VALUE_TYPE_INST, "something is wrong");

    // Do not lookup recursively
    SnukEnv *env = snuk_scope_lookup(type_or_inst.type_value.closure, field);
    if (!env && type_or_inst.type_value.type_scope) {
        env = snuk_scope_lookup(type_or_inst.type_value.type_scope, field);
        // Add the new member to instance
        if (env) {
            if (!snuk_interpreter_value_is_of_type(intpret, value, env->type)) return false;
            SnukEnv *inst_env = snuk_env_create(env->name, env->type, value);
            if (!snuk_scope_add_env(type_or_inst.type_value.closure, inst_env)) return false;
            return true;
        }
    }
    if (!env) return false;

    if (!snuk_interpreter_value_is_of_type(intpret, value, env->type)) return false;
    snuk_env_assign_value(env, value);
    return true;
}

SNUK_INLINE void interpreter_trash(SnukInterpreter *intpret, SnukValue value) {
    snuk_darray_push(&intpret->trash, value);
}

SNUK_INLINE void interpreter_clear_trash(SnukInterpreter *intpret) {
    uint64_t count = snuk_darray_get_length(intpret->trash);
    for (uint64_t i = 0; i < count; ++i) snuk_value_free(intpret->trash[i]);
    snuk_darray_clear(&intpret->trash);
}

