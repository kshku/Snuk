#include "snuk/interpreter/native.h"

#include "snuk/darray.h"
#include "snuk/interpreter/builtins/snuk_builtins.h"
#include "snuk/interpreter/interpreter_helper.h"

SnukValue snuk_native_lookup(SnukInterpreter *intpret, const char *name) {
    return snuk_interpreter_get_env(intpret, snuk_string_view_create(name));
}

SnukValue snuk_native_get_member(SnukInterpreter *intpret, SnukValue type_or_inst, const char *name) {
    SnukStringView name_sv = snuk_string_view_create(name);
    SnukValue res;
    bool should_trash = false;
    if (type_or_inst.type == SNUK_VALUE_TYPE || type_or_inst.type == SNUK_VALUE_TYPE_INST) {
        res = interpreter_get_member(intpret, type_or_inst, name_sv);
    } else if (type_or_inst.type == SNUK_VALUE_NULL) {
        res = builtin_null_get_member(intpret, name_sv);
    } else {
        SnukTypeMember members[] = {
            {.name = "value", .value = snuk_value_copy(type_or_inst)},
        };
        switch (type_or_inst.type) {
            case SNUK_VALUE_INT:
                members[0].type = &int_type;
                break;
            case SNUK_VALUE_FLOAT:
                members[0].type = &float_type;
                break;
            case SNUK_VALUE_BOOL:
                members[0].type = &bool_type;
                break;
            case SNUK_VALUE_STRING:
                members[0].type = &str_type;
                break;
            default:
                SNUK_SHOULD_NOT_REACH_HERE;
                break;
        }

        type_or_inst = snuk_native_create_inst(intpret, members[0].type, members, 1, true);
        should_trash = true;

        res = interpreter_get_member(intpret, type_or_inst, name_sv);
    }

    // insert the instance scope
    if (type_or_inst.type == SNUK_VALUE_TYPE_INST) {
        if (res.type == SNUK_VALUE_FN)
            res.fn_value.instance = snuk_ref_counter_retain_weak(type_or_inst.type_value.closure);
        else if (res.type == SNUK_VALUE_FN_NATIVE)
            res.native_fn.instance = snuk_ref_counter_retain_weak(type_or_inst.type_value.closure);
    }

    if (should_trash) interpreter_trash(intpret, type_or_inst);

    return res;
}

SnukValue snuk_native_call_function(SnukInterpreter *intpret, SnukValue fn, SnukParameter *params, uint64_t count) {
    if (fn.type != SNUK_VALUE_FN && fn.type != SNUK_VALUE_FN_NATIVE)
        return (SnukValue){.type = SNUK_VALUE_UNKOWN};

    SnukRefCounter *fn_scope_rc = NULL;
    if (fn.type == SNUK_VALUE_FN) fn_scope_rc = fn.fn_value.closure;
    else fn_scope_rc = fn.native_fn.closure;

    SnukRefCounter *prev_instance = snuk_ref_counter_move(&intpret->instance);
    if (fn.type == SNUK_VALUE_FN) {
        if (fn.fn_value.instance) intpret->instance = snuk_ref_counter_retain(fn.fn_value.instance);
    } else if (fn.native_fn.instance) {
        intpret->instance = snuk_ref_counter_retain(fn.native_fn.instance);
    }

    interpreter_push_scope(intpret);

    SnukScope *fn_scope = GET_SCOPE(fn_scope_rc);
    uint64_t fn_param_count = snuk_darray_get_length(fn_scope->vars);

    if (fn_param_count < count) return (SnukValue){.type = SNUK_VALUE_UNKOWN};
    for (uint64_t i = 0; i < count; ++i) {
        SnukStringView name = snuk_string_view_create(params[i].name);
        SnukEnv *fn_env = snuk_scope_lookup(fn_scope_rc, name);
        if (!fn_env) return (SnukValue){.type = SNUK_VALUE_UNKOWN};
        SnukValue value;
        if (params[i].build_value) value = params[i].build_value(intpret, true);
        else value = params[i].value;
        if (!snuk_interpreter_create_env(intpret, name, fn_env->type, value, false))
            return (SnukValue){.type = SNUK_VALUE_UNKOWN};
        snuk_value_free(value);
    }

    for (uint64_t i = 0; i < fn_param_count; ++i) {
        SnukEnv *fn_env = fn_scope->vars[i];
        SnukEnv *env = snuk_scope_lookup(intpret->current, fn_env->name);
        if (!env) {
            if (fn_env->value.type == SNUK_VALUE_UNKOWN)
                return (SnukValue){.type = SNUK_VALUE_UNKOWN};
            if (!snuk_interpreter_create_env(intpret, fn_env->name, fn_env->type, fn_env->value, false))
                return (SnukValue){.type = SNUK_VALUE_UNKOWN};
        }
    }

    SnukRefCounter *new_scope = snuk_ref_counter_retain(intpret->current);

    interpreter_pop_scope(intpret);

    snuk_scope_set_parent(new_scope, snuk_ref_counter_retain(fn_scope_rc), false);

    SnukRefCounter *temp = snuk_ref_counter_move(&intpret->current);
    intpret->current = snuk_ref_counter_move(&new_scope);

    SnukValue ret;
    if (fn.type == SNUK_VALUE_FN)
        ret = execute_block_expr(intpret, fn.fn_value.body, SNUK_SIGNAL_RETURN, SNUK_SIGNAL_NONE, false);
    else ret = fn.native_fn.fn(intpret);

    new_scope = snuk_ref_counter_move(&intpret->current);
    intpret->current = snuk_ref_counter_move(&temp);

    snuk_ref_counter_release(&new_scope);

    if (intpret->instance) snuk_ref_counter_release(&intpret->instance);
    intpret->instance = snuk_ref_counter_move(&prev_instance);

    return ret;
}

SnukValue snuk_native_create_type(SnukInterpreter *intpret, SnukTypeMember *members, uint64_t count, bool weak_ref) {
    interpreter_push_scope(intpret);

    for (uint64_t i = 0; i < count; ++i) {
        SnukValue value;
        if (members[i].build_value) value = members[i].build_value(intpret, true);
        else value = members[i].value;
        if (!snuk_native_add_value(intpret, members[i].name, members[i].type, value, members[i].is_const))
            return (SnukValue){.type = SNUK_VALUE_UNKOWN};
        snuk_value_free(value);
    }

    SnukValue type = {
        .type = SNUK_VALUE_TYPE,
        .type_value = {
            .type_scope=NULL,
            .closure = snuk_ref_counter_retain(intpret->current),
            .weak_ref = false,
            .type = &type_type,
        },
    };

    interpreter_pop_scope(intpret);

    if (weak_ref) snuk_scope_downgrade_parent(type.type_value.closure);

    return type;
}

SnukValue snuk_native_create_fn(SnukInterpreter *intpret, SnukParameter *params, uint64_t count,
                                SnukType *fn_type, native_function_t fn, bool weak_ref) {
    interpreter_push_scope(intpret);

    uint64_t param_count = snuk_darray_get_length(fn_type->fn.param_types);
    if (count != param_count) return (SnukValue){.type = SNUK_VALUE_UNKOWN};

    for (uint64_t i = 0; i < count; ++i) {
        SnukValue value;
        if (params[i].build_value) value = params[i].build_value(intpret, true);
        else value = params[i].value;
        if (!snuk_native_add_value(intpret, params[i].name, fn_type->fn.param_types[i], value, false))
            return (SnukValue){.type = SNUK_VALUE_UNKOWN};
        snuk_value_free(value);
    }

    SnukValue function = {
        .type = SNUK_VALUE_FN_NATIVE,
        .native_fn = {
            .closure = snuk_ref_counter_retain(intpret->current),
            .type = fn_type,
            .fn = fn,
        },
    };

    interpreter_pop_scope(intpret);

    if (weak_ref) snuk_scope_downgrade_parent(function.native_fn.closure);

    return function;
}

SnukValue snuk_native_create_inst(
    SnukInterpreter *intpret, SnukType *type, SnukTypeMember *members, uint64_t count, bool weak_ref) {
    SnukEnv *env = interpreter_lookup(intpret, type->name);
    if (env->value.type != SNUK_VALUE_TYPE) return (SnukValue){.type = SNUK_VALUE_UNKOWN};

    interpreter_push_scope(intpret);

    SnukValue inst = {
        .type = SNUK_VALUE_TYPE_INST,
        .type_value = {
            .type = type,
            .closure = snuk_ref_counter_retain(intpret->current),
            .weak_ref = false,
            .type_scope = snuk_ref_counter_retain(env->value.type_value.closure),
        },
    };

    for (uint64_t i = 0; i < count; ++i) {
        SnukStringView name = snuk_string_view_create(members[i].name);
        SnukValue value;
        if (members[i].build_value) value = members[i].build_value(intpret, true);
        else value = members[i].value;
        SnukValueType val_type = snuk_builtins_get_value_type(value.type_value.type->name);
        if (val_type != SNUK_VALUE_UNKOWN && snuk_string_view_equal(name, value_str) && value.type != val_type)
            return (SnukValue){.type = SNUK_VALUE_UNKOWN};
        if (!interpreter_set_member(intpret, value, name, value))
            return (SnukValue){.type = SNUK_VALUE_UNKOWN};
        snuk_value_free(value);
    }

    SnukValue self_value = snuk_value_copy(inst);
    snuk_ref_counter_downgrade(self_value.type_value.closure);
    self_value.type_value.weak_ref = true;

    if (!snuk_interpreter_create_env(intpret, self_str, self_value.type_value.type, self_value, true))
        return (SnukValue){.type = SNUK_VALUE_UNKOWN};

    snuk_value_free(self_value);

    interpreter_pop_scope(intpret);

    if (weak_ref) snuk_scope_downgrade_parent(inst.type_value.closure);

    return inst;
}

SnukValue snuk_native_create_int(SnukInterpreter *intpret, int64_t value, bool weak_ref) {
    SNUK_UNUSED(intpret);
    SNUK_UNUSED(weak_ref);
    return (SnukValue){
        .type = SNUK_VALUE_INT,
        .int_value = value,
    };
}

SnukValue snuk_native_create_float(SnukInterpreter *intpret, double value, bool weak_ref) {
    SNUK_UNUSED(intpret);
    SNUK_UNUSED(weak_ref);
    return (SnukValue){
        .type = SNUK_VALUE_FLOAT,
        .float_value = value,
    };
}

SnukValue snuk_native_create_bool(SnukInterpreter *intpret, bool value, bool weak_ref) {
    SNUK_UNUSED(intpret);
    SNUK_UNUSED(weak_ref);
    return (SnukValue){
        .type = SNUK_VALUE_BOOL,
        .bool_value = value,
    };
}

SnukValue snuk_native_create_null(SnukInterpreter *intpret, bool weak_ref) {
    SNUK_UNUSED(intpret);
    SNUK_UNUSED(weak_ref);
    return (SnukValue){
        .type = SNUK_VALUE_NULL,
    };
}

SnukValue snuk_native_create_interface(SnukInterpreter *intpret, SnukType *interface, bool weak_ref) {
    SNUK_UNUSED(intpret);
    SNUK_UNUSED(weak_ref);
    return (SnukValue){
        .type = SNUK_VALUE_INTERFACE,
        .interface = {
            .type = interface,
        },
    };
}

SnukValue snuk_native_create_string(SnukInterpreter *intpret, const char *str, bool weak_ref) {
    SNUK_UNUSED(intpret);
    SNUK_UNUSED(weak_ref);
    return (SnukValue){
        .type = SNUK_VALUE_STRING,
        .string_value = snuk_string_view_create(str),
    };
}
