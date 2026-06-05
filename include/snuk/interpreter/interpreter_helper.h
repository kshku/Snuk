#pragma once

#include "error_code.h"
#include "interpreter.h"
#include "snuk/darray.h"
#include "snuk/defines.h"
#include "snuk_scope.h"

SNUK_INLINE void interpreter_error(SnukInterpreter *intpret, SnukErrorCode err_code) {
    if (intpret->err_code != SNUK_ERROR_NONE) return;
    intpret->err_code = err_code;
}

/**
 * @brief Push a new child scope and make it the interpreter's current scope.
 */
SNUK_INLINE void interpreter_push_scope(SnukInterpreter *intpret) {
    intpret->current = snuk_scope_create(snuk_ref_counter_move(&intpret->current), false, false);
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

SNUK_INLINE SnukEnv *interpreter_get_member_env(
    SnukInterpreter *intpret, SnukValue type_or_inst, SnukStringView field, bool *locked) {
    SNUK_UNUSED(intpret);
    if (type_or_inst.type != SNUK_VALUE_TYPE && type_or_inst.type != SNUK_VALUE_TYPE_INST)
        return NULL;

    // Do not lookup recursively
    SnukEnv *env = snuk_scope_lookup(type_or_inst.type_value.closure, field, locked);
    if (!env && type_or_inst.type_value.type_scope)
        env = snuk_scope_lookup(type_or_inst.type_value.type_scope, field, locked);
    return env;
}

SNUK_INLINE SnukValue interpreter_get_member(
    SnukInterpreter *intpret, SnukValue type_or_inst, SnukStringView field, bool *locked) {
    SnukEnv *env = interpreter_get_member_env(intpret, type_or_inst, field, locked);
    if (!env) return (SnukValue){.type = SNUK_VALUE_UNKOWN};
    return snuk_value_copy(env->value);
}

SNUK_INLINE bool interpreter_set_member(
    SnukInterpreter *intpret, SnukValue inst, SnukStringView field, SnukValue value) {
    if (inst.type != SNUK_VALUE_TYPE_INST) return false;

    // Do not lookup recursively
    bool locked;
    SnukEnv *env = snuk_scope_lookup(inst.type_value.closure, field, &locked);
    if (!env && inst.type_value.type_scope) {
        env = snuk_scope_lookup(inst.type_value.type_scope, field, &locked);
        // type scope must be locked
        SNUK_ASSERT(locked, "type scope isn't locked");
        // Add the new member to instance
        if (env) {
            if (!snuk_interpreter_value_is_of_type(intpret, value, env->type)) return false;
            SnukEnv *inst_env = snuk_env_create(env->name, env->type, value, env->is_const);
            if (!snuk_scope_add_env(inst.type_value.closure, inst_env)) return false;
            return true;
        }
    }
    if (!env) return false;
    if (locked) return false;

    if (!snuk_interpreter_value_is_of_type(intpret, value, env->type)) return false;
    return snuk_env_assign_value(env, value);
}

/**
 * @brief Walk the scope chain from current to global to resolve a name.
 *
 * Do not use the returned env to set value
 */
SNUK_INLINE SnukEnv *interpreter_lookup(SnukInterpreter *intpret, SnukStringView name, bool *locked) {
    SnukEnv *env = NULL;
    if (intpret->instance) {
        SnukEnv *self_env = snuk_scope_lookup(intpret->instance, self_str, locked);
        if (!self_env) return NULL;
        env = interpreter_get_member_env(intpret, self_env->value, name, locked);
        if (env) return env;
    }
    if (!env) env = snuk_scope_lookup_recursive(intpret->current, name, locked);
    return env;
}

SNUK_INLINE void interpreter_trash(SnukInterpreter *intpret, SnukValue value) {
    snuk_darray_push(&intpret->trash, value);
}

SNUK_INLINE void interpreter_clear_trash(SnukInterpreter *intpret) {
    uint64_t count = snuk_darray_get_length(intpret->trash);
    for (uint64_t i = 0; i < count; ++i) snuk_value_free(intpret->trash[i]);
    snuk_darray_clear(&intpret->trash);
}

SnukValue execute_block_expr(
    SnukInterpreter *intpret, SnukExpr *block, int capture_signals, int propogate_signals, bool weak_ref);
