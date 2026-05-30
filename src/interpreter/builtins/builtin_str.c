#include "snuk/interpreter/builtins/snuk_builtins.h"

#include <stdio.h>

static SnukValue to_int(SnukInterpreter *intpret);
static SnukValue to_float(SnukInterpreter *intpret);
static SnukValue to_bool(SnukInterpreter *intpret);
static SnukValue to_str(SnukInterpreter *intpret);
static SnukValue length(SnukInterpreter *intpret);
static SnukValue get(SnukInterpreter *intpret);

static SnukValue build_to_int(SnukInterpreter *intpret);
static SnukValue build_to_float(SnukInterpreter *intpret);
static SnukValue build_to_bool(SnukInterpreter *intpret);
static SnukValue build_to_str(SnukInterpreter *intpret);
static SnukValue build_length(SnukInterpreter *intpret);
static SnukValue build_get(SnukInterpreter *intpret);

static BuiltinMember str_members[] = {
    {.field = "to_int",   .build_field = build_to_int  },
    {.field = "to_float", .build_field = build_to_float},
    {.field = "to_bool",  .build_field = build_to_bool },
    {.field = "to_str",   .build_field = build_to_str  },
    {.field = "length",   .build_field = build_length  },
    {.field = "get",      .build_field = build_get     },
};

SnukType str_type = {
    .type = TYPE_NAMED,
    .name = {.str = "str", .len = 3}
};

SnukValue builtin_str_create_type(SnukInterpreter *intpret, bool weak_ref) {
    interpreter_push_scope(intpret);

    SNUK_ASSERT(snuk_interpreter_create_env(intpret, value_str, &any_type, (SnukValue){.type = SNUK_VALUE_NULL}, false),
                "something went wrong");

    for (uint64_t i = 0; i < SNUK_ARRAY_LENGTH(str_members); ++i) {
        SnukStringView name = snuk_string_view_create(str_members[i].field);
        SnukValue member = str_members[i].build_field(intpret);
        SnukType *type = NULL;
        switch (member.type) {
            case SNUK_VALUE_FN_BUILTIN:
                type = member.builtin_fn.type;
                break;

            default:
                SNUK_SHOULD_NOT_REACH_HERE;
                break;
        }
        SNUK_ASSERT(type, "something went wrong");

        SNUK_ASSERT(snuk_interpreter_create_env(intpret, name, type, member, false),
                    "something went wrong");
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

static SnukValue build_length(SnukInterpreter *intpret) {
    SnukRefCounter *scope = snuk_scope_create(snuk_ref_counter_retain_weak(intpret->current), true);

    static SnukType type;
    type = (SnukType){
        .type = TYPE_FN,
        .fn ={
            .param_types = snuk_darray_create(SnukType *, &intpret->allocator),
            .return_type = &int_type,
        },
    };

    return (SnukValue){
        .type = SNUK_VALUE_FN_BUILTIN,
        .builtin_fn = {
            .closure = scope,
            .fn = length,
            .type = &type,
        },
    };
}

static SnukValue build_get(SnukInterpreter *intpret) {
    SnukRefCounter *scope = snuk_scope_create(snuk_ref_counter_retain_weak(intpret->current), true);
    Parameters params[] = {
        {
         .name = snuk_string_view_create_with_len("start", 5),
         .type = &int_type,
         .value = (SnukValue){.type = SNUK_VALUE_INT, .int_value = 0},
         },
        {
         .name = snuk_string_view_create_with_len("len",                                                3),
         .type = &int_type,
         .value = (SnukValue){.type = SNUK_VALUE_NULL},
         },
    };

    add_paramters(scope, params, SNUK_ARRAY_LENGTH(params));

    static SnukType type;
    type = (SnukType){
        .type = TYPE_FN,
        .fn ={
            .param_types = snuk_darray_create(SnukType *, &intpret->allocator),
            .return_type = &str_type,
        },
    };

    snuk_darray_push(&type.fn.param_types, &int_type);
    snuk_darray_push(&type.fn.param_types, &int_type);

    return (SnukValue){
        .type = SNUK_VALUE_FN_BUILTIN,
        .builtin_fn = {
            .closure = scope,
            .fn = get,
            .type = &type,
        },
    };
}

static SnukValue to_int(SnukInterpreter *intpret) {
    SnukEnv *value_env = interpreter_lookup(intpret, value_str);
    SNUK_ASSERT(value_env, "something went wrong!");
    SNUK_ASSERT(value_env->value.type == SNUK_VALUE_STRING || value_env->value.type == SNUK_VALUE_NULL,
                "unexpected value");

    if (value_env->value.type == SNUK_VALUE_NULL)
        return (SnukValue){
            .type = SNUK_VALUE_INT,
            .int_value = 0,
        };

    char *str = snuk_string_view_get_cstr(value_env->value.string_value);

    int64_t int_value = 0;
    if (sscanf(str, "\"%" PRId64 "\"", &int_value) == 0)
        return (SnukValue){.type = SNUK_VALUE_NULL};

    return (SnukValue){
        .type = SNUK_VALUE_INT,
        .int_value = int_value,
    };
}

static SnukValue to_float(SnukInterpreter *intpret) {
    SnukEnv *value_env = interpreter_lookup(intpret, value_str);
    SNUK_ASSERT(value_env, "something went wrong!");
    SNUK_ASSERT(value_env->value.type == SNUK_VALUE_STRING || value_env->value.type == SNUK_VALUE_NULL,
                "unexpected value");

    if (value_env->value.type == SNUK_VALUE_NULL)
        return (SnukValue){
            .type = SNUK_VALUE_FLOAT,
            .float_value = 0.0,
        };

    char *str = snuk_string_view_get_cstr(value_env->value.string_value);

    double float_value = 0;
    if (sscanf(str, "\"%lf\"", &float_value) == 0) return (SnukValue){.type = SNUK_VALUE_NULL};

    return (SnukValue){
        .type = SNUK_VALUE_FLOAT,
        .float_value = float_value,
    };
}

static SnukValue to_bool(SnukInterpreter *intpret) {
    SnukEnv *value_env = interpreter_lookup(intpret, value_str);
    SNUK_ASSERT(value_env, "something went wrong!");
    SNUK_ASSERT(value_env->value.type == SNUK_VALUE_STRING || value_env->value.type == SNUK_VALUE_NULL,
                "unexpected value");
    // Return true if non-empty string
    return (SnukValue){
        .type = SNUK_VALUE_BOOL,
        .bool_value
        = value_env->value.type == SNUK_VALUE_NULL ? false : value_env->value.string_value.len != 0,
    };
}

static SnukValue to_str(SnukInterpreter *intpret) {
    SnukEnv *value_env = interpreter_lookup(intpret, value_str);
    SNUK_ASSERT(value_env, "something went wrong!");
    SNUK_ASSERT(value_env->value.type == SNUK_VALUE_STRING || value_env->value.type == SNUK_VALUE_NULL,
                "unexpected value");
    if (value_env->value.type == SNUK_VALUE_NULL)
        return (SnukValue){
            .type = SNUK_VALUE_STRING,
            .string_value = snuk_string_view_create_with_len("\"null\"", 6),
        };
    return snuk_value_copy(value_env->value);
}

static SnukValue length(SnukInterpreter *intpret) {
    SnukEnv *value_env = interpreter_lookup(intpret, value_str);
    SNUK_ASSERT(value_env, "something went wrong!");
    SNUK_ASSERT(value_env->value.type == SNUK_VALUE_STRING || value_env->value.type == SNUK_VALUE_NULL,
                "unexpected value");
    return (SnukValue){
        .type = SNUK_VALUE_INT,
        .int_value
        = value_env->value.type == SNUK_VALUE_NULL ? 0 : (int64_t)value_env->value.string_value.len - 2,
    };
}

static SnukValue get(SnukInterpreter *intpret) {
    SnukEnv *value_env = interpreter_lookup(intpret, value_str);
    SNUK_ASSERT(value_env, "something went wrong!");
    SNUK_ASSERT(value_env->value.type == SNUK_VALUE_STRING || value_env->value.type == SNUK_VALUE_NULL,
                "unexpected value");

    if (value_env->value.type == SNUK_VALUE_NULL) return snuk_value_copy(value_env->value);

    SnukEnv *envs[2];
    SnukStringView names[] = {
        snuk_string_view_create_with_len("start", 5),
        snuk_string_view_create_with_len("len", 3),
    };
    get_parameters(intpret->current, names, envs, 2);

    int64_t start = envs[0]->value.int_value;
    SnukStringView string = value_env->value.string_value;
    int64_t len;
    if (envs[1]->value.type == SNUK_VALUE_NULL) len = (int64_t)string.len - 2;
    else len = envs[1]->value.int_value;

    if (start < 0 || start >= (int64_t)string.len - 2) goto fail;
    if (len < 0 || start + len > (int64_t)string.len - 2) goto fail;

    char *new_str = snuk_alloc((len + 2) * sizeof(char), alignof(char));
    new_str[0] = '"';
    memcpy(new_str + 1, string.str + start + 1, len);
    new_str[len + 1] = '"';
    return (SnukValue){
        .type = SNUK_VALUE_STRING,
        .string_value = snuk_string_view_create_with_len(new_str, len + 2),
    };

fail:
    return (SnukValue){.type = SNUK_VALUE_NULL};
}
