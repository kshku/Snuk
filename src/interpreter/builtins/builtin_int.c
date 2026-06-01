#include "snuk/interpreter/builtins/snuk_builtins.h"

#include <stdio.h>

static SnukValue to_int(SnukInterpreter *intpret);
static SnukValue to_float(SnukInterpreter *intpret);
static SnukValue to_bool(SnukInterpreter *intpret);
static SnukValue to_str(SnukInterpreter *intpret);

static SnukValue build_to_int(SnukInterpreter *intpret);
static SnukValue build_to_float(SnukInterpreter *intpret);
static SnukValue build_to_bool(SnukInterpreter *intpret);
static SnukValue build_to_str(SnukInterpreter *intpret);

static BuiltinMember int_members[] = {
    {.field = "to_int",   .build_field = build_to_int  },
    {.field = "to_float", .build_field = build_to_float},
    {.field = "to_bool",  .build_field = build_to_bool },
    {.field = "to_str",   .build_field = build_to_str  },
};

SnukType int_type = {
    .type = TYPE_NAMED,
    .name = {.str = "int", .len = 3}
};

SnukValue builtin_int_create_type(SnukInterpreter *intpret, bool weak_ref) {
    interpreter_push_scope(intpret);

    if (!(snuk_interpreter_create_env(intpret, value_str, &any_type, (SnukValue){.type = SNUK_VALUE_NULL}, false)))
        return (SnukValue){.type = SNUK_VALUE_UNKOWN};

    for (uint64_t i = 0; i < SNUK_ARRAY_LENGTH(int_members); ++i) {
        SnukStringView name = snuk_string_view_create(int_members[i].field);
        SnukValue member = int_members[i].build_field(intpret);
        SnukType *type = NULL;
        switch (member.type) {
            case SNUK_VALUE_FN_BUILTIN:
                type = member.builtin_fn.type;
                break;

            default:
                SNUK_SHOULD_NOT_REACH_HERE;
                break;
        }
        if (!(type)) return (SnukValue){.type = SNUK_VALUE_UNKOWN};

        if (!(snuk_interpreter_create_env(intpret, name, type, member, false)))
            return (SnukValue){.type = SNUK_VALUE_UNKOWN};
        snuk_value_free(member);
    }

    SnukValue value = {
        .type = SNUK_VALUE_TYPE,
        .type_value = {
            .type_scope = NULL,
            .closure = snuk_ref_counter_retain(intpret->current),
            .weak_ref = false,
            .type = &type_type,
        },
    };

    interpreter_pop_scope(intpret);

    if (weak_ref) snuk_scope_downgrade_parent(value.type_value.closure);

    return value;
}

static SnukValue build_to_int(SnukInterpreter *intpret) {
    SnukRefCounter *scope = snuk_scope_create(snuk_ref_counter_retain_weak(intpret->current), true);

    // called only once, so no problem
    static SnukType type;
    type = (SnukType){
        .type = TYPE_FN,
        .fn = {
            .param_types = snuk_darray_create(SnukType *, &intpret->allocator),
            .return_type = &int_type,
        },
    };

    return (SnukValue){
        .type = SNUK_VALUE_FN_BUILTIN,
        .builtin_fn = {
            .closure = snuk_ref_counter_move(&scope),
            .fn = to_int,
            .type = &type,
        },
    };
}

static SnukValue build_to_float(SnukInterpreter *intpret) {
    SnukRefCounter *scope = snuk_scope_create(snuk_ref_counter_retain_weak(intpret->current), true);

    static SnukType type;
    type = (SnukType){
        .type = TYPE_FN,
        .fn = {
            .param_types = snuk_darray_create(SnukType *, &intpret->allocator),
            .return_type = &float_type,
        },
    };

    return (SnukValue){
        .type = SNUK_VALUE_FN_BUILTIN,
        .builtin_fn = {
            .closure = snuk_ref_counter_move(&scope),
            .fn = to_float,
            .type = &type,
        },
    };
}

static SnukValue build_to_bool(SnukInterpreter *intpret) {
    SnukRefCounter *scope = snuk_scope_create(snuk_ref_counter_retain_weak(intpret->current), true);

    static SnukType type;
    type = (SnukType){
        .type = TYPE_FN,
        .fn ={
            .param_types = snuk_darray_create(SnukType *, &intpret->allocator),
            .return_type = &bool_type,
        },
    };

    return (SnukValue){
        .type = SNUK_VALUE_FN_BUILTIN,
        .builtin_fn = {
            .closure = snuk_ref_counter_move(&scope),
            .fn = to_bool,
            .type = &type,
        },
    };
}

static SnukValue build_to_str(SnukInterpreter *intpret) {
    SnukRefCounter *scope = snuk_scope_create(snuk_ref_counter_retain_weak(intpret->current), true);

    static SnukType type;
    type = (SnukType){
        .type = TYPE_FN,
        .fn ={
            .param_types = snuk_darray_create(SnukType *, &intpret->allocator),
            .return_type = &str_type,
        },
    };

    return (SnukValue){
        .type = SNUK_VALUE_FN_BUILTIN,
        .builtin_fn = {
            .closure = snuk_ref_counter_move(&scope),
            .fn = to_str,
            .type = &type,
        },
    };
}

static SnukValue to_int(SnukInterpreter *intpret) {
    SnukEnv *value_env = interpreter_lookup(intpret, value_str);
    if (!value_env) return (SnukValue){.type = SNUK_VALUE_UNKOWN};
    if (!(value_env->value.type == SNUK_VALUE_INT || value_env->value.type == SNUK_VALUE_NULL))
        return (SnukValue){.type = SNUK_VALUE_UNKOWN};
    if (value_env->value.type == SNUK_VALUE_NULL)
        return (SnukValue){
            .type = SNUK_VALUE_INT,
            .int_value = 0,
        };
    return snuk_value_copy(value_env->value);
}

static SnukValue to_float(SnukInterpreter *intpret) {
    SnukEnv *value_env = interpreter_lookup(intpret, value_str);
    if (!value_env) return (SnukValue){.type = SNUK_VALUE_UNKOWN};
    if (!(value_env->value.type == SNUK_VALUE_INT || value_env->value.type == SNUK_VALUE_NULL))
        return (SnukValue){.type = SNUK_VALUE_UNKOWN};
    return (SnukValue){
        .type = SNUK_VALUE_FLOAT,
        .float_value = value_env->value.type == SNUK_VALUE_NULL ? 0.0 : (double)value_env->value.int_value,
    };
}

static SnukValue to_bool(SnukInterpreter *intpret) {
    SnukEnv *value_env = interpreter_lookup(intpret, value_str);
    if (!value_env) return (SnukValue){.type = SNUK_VALUE_UNKOWN};
    if (!(value_env->value.type == SNUK_VALUE_INT || value_env->value.type == SNUK_VALUE_NULL))
        return (SnukValue){.type = SNUK_VALUE_UNKOWN};
    return (SnukValue){
        .type = SNUK_VALUE_BOOL,
        .bool_value = value_env->value.type == SNUK_VALUE_NULL ? false : (bool)value_env->value.int_value,
    };
}

static SnukValue to_str(SnukInterpreter *intpret) {
    SnukEnv *value_env = interpreter_lookup(intpret, value_str);
    if (!value_env) return (SnukValue){.type = SNUK_VALUE_UNKOWN};
    if (!(value_env->value.type == SNUK_VALUE_INT || value_env->value.type == SNUK_VALUE_NULL))
        return (SnukValue){.type = SNUK_VALUE_UNKOWN};
    if (value_env->value.type == SNUK_VALUE_NULL) {
        return (SnukValue){
            .type = SNUK_VALUE_STRING,
            .string_value = snuk_string_view_create_with_len("\"null\"", 6),
        };
    }
    char *buf = (char *)snuk_alloc(25 * sizeof(char), alignof(char));
    uint64_t len = 0;
    len = snprintf(buf, 25, "\"%" PRId64 "\"", value_env->value.int_value);
    buf[len] = 0;
    return (SnukValue){
        .type = SNUK_VALUE_STRING,
        .string_value = snuk_string_view_create_with_len(buf, len),
    };
}
