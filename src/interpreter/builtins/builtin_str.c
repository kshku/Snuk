#include "builtin_common.h"
#include "snuk/interpreter/builtins/snuk_builtins.h"

#include <stdio.h>

static SnukValue to_int(SnukInterpreter *intpret);
static SnukValue to_float(SnukInterpreter *intpret);
static SnukValue to_bool(SnukInterpreter *intpret);
static SnukValue to_str(SnukInterpreter *intpret);
static SnukValue length(SnukInterpreter *intpret);
static SnukValue get(SnukInterpreter *intpret);

static SnukValue build_to_int(SnukInterpreter *intpret, bool weak_ref);
static SnukValue build_to_float(SnukInterpreter *intpret, bool weak_ref);
static SnukValue build_to_bool(SnukInterpreter *intpret, bool weak_ref);
static SnukValue build_to_str(SnukInterpreter *intpret, bool weak_ref);
static SnukValue build_length(SnukInterpreter *intpret, bool weak_ref);
static SnukValue build_get(SnukInterpreter *intpret, bool weak_ref);

SnukTypeMember str_members[] = {
    {.name = "value",    .type = &any_type,        .value = {.type = SNUK_VALUE_NULL}, .is_const = false},
    {.name = "to_int",   .type = &to_int_type,     .build_value = build_to_int,        .is_const = false},
    {.name = "to_float", .type = &to_float_type,   .build_value = build_to_float,      .is_const = false},
    {.name = "to_bool",  .type = &to_bool_type,    .build_value = build_to_bool,       .is_const = false},
    {.name = "to_str",   .type = &to_str_type,     .build_value = build_to_str,        .is_const = false},
    {.name = "length",   .type = &str_length_type, .build_value = build_length,        .is_const = false},
    {.name = "get",      .type = &str_get_type,    .build_value = build_get,           .is_const = false},
};

SnukValue builtin_str_create_type(SnukInterpreter *intpret, bool weak_ref) {
    return snuk_native_create_type(intpret, str_members, SNUK_ARRAY_LENGTH(str_members), weak_ref);
}

SnukType str_type = {
    .type = TYPE_NAMED,
    .name = {.str = "str", .len = 3}
};

static SnukValue build_to_int(SnukInterpreter *intpret, bool weak_ref) {
    return snuk_native_create_fn(intpret, NULL, 0, &to_int_type, to_int, weak_ref);
}

static SnukValue build_to_float(SnukInterpreter *intpret, bool weak_ref) {
    return snuk_native_create_fn(intpret, NULL, 0, &to_float_type, to_float, weak_ref);
}

static SnukValue build_to_bool(SnukInterpreter *intpret, bool weak_ref) {
    return snuk_native_create_fn(intpret, NULL, 0, &to_bool_type, to_bool, weak_ref);
}

static SnukValue build_to_str(SnukInterpreter *intpret, bool weak_ref) {
    return snuk_native_create_fn(intpret, NULL, 0, &to_str_type, to_str, weak_ref);
}

static SnukValue build_length(SnukInterpreter *intpret, bool weak_ref) {
    return snuk_native_create_fn(intpret, NULL, 0, &str_length_type, length, weak_ref);
}

static SnukValue build_get(SnukInterpreter *intpret, bool weak_ref) {
    SnukParameter params[] = {
        {
         .name = "start",
         .value = {.type = SNUK_VALUE_INT, .int_value = 0},
         },
        {
         .name = "len",
         .value = {.type = SNUK_VALUE_NULL},
         },
    };
    return snuk_native_create_fn(intpret, params, SNUK_ARRAY_LENGTH(params), &str_get_type, get, weak_ref);
}

static SnukValue to_int(SnukInterpreter *intpret) {
    SnukEnv *value_env = interpreter_lookup(intpret, value_str);
    if (!value_env) return (SnukValue){.type = SNUK_VALUE_UNKOWN};
    if (!(value_env->value.type == SNUK_VALUE_STRING || value_env->value.type == SNUK_VALUE_NULL))
        return (SnukValue){.type = SNUK_VALUE_UNKOWN};

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
    if (!value_env) return (SnukValue){.type = SNUK_VALUE_UNKOWN};
    if (!(value_env->value.type == SNUK_VALUE_STRING || value_env->value.type == SNUK_VALUE_NULL))
        return (SnukValue){.type = SNUK_VALUE_UNKOWN};

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
    if (!value_env) return (SnukValue){.type = SNUK_VALUE_UNKOWN};
    if (!(value_env->value.type == SNUK_VALUE_STRING || value_env->value.type == SNUK_VALUE_NULL))
        return (SnukValue){.type = SNUK_VALUE_UNKOWN};
    // Return true if non-empty string
    return (SnukValue){
        .type = SNUK_VALUE_BOOL,
        .bool_value
        = value_env->value.type == SNUK_VALUE_NULL ? false : value_env->value.string_value.len != 0,
    };
}

static SnukValue to_str(SnukInterpreter *intpret) {
    SnukEnv *value_env = interpreter_lookup(intpret, value_str);
    if (!value_env) return (SnukValue){.type = SNUK_VALUE_UNKOWN};
    if (!(value_env->value.type == SNUK_VALUE_STRING || value_env->value.type == SNUK_VALUE_NULL))
        return (SnukValue){.type = SNUK_VALUE_UNKOWN};
    if (value_env->value.type == SNUK_VALUE_NULL)
        return (SnukValue){
            .type = SNUK_VALUE_STRING,
            .string_value = snuk_string_view_create_with_len("\"null\"", 6),
        };
    return snuk_value_copy(value_env->value);
}

static SnukValue length(SnukInterpreter *intpret) {
    SnukEnv *value_env = interpreter_lookup(intpret, value_str);
    if (!value_env) return (SnukValue){.type = SNUK_VALUE_UNKOWN};
    if (!(value_env->value.type == SNUK_VALUE_STRING || value_env->value.type == SNUK_VALUE_NULL))
        return (SnukValue){.type = SNUK_VALUE_UNKOWN};
    return (SnukValue){
        .type = SNUK_VALUE_INT,
        .int_value
        = value_env->value.type == SNUK_VALUE_NULL ? 0 : (int64_t)value_env->value.string_value.len - 2,
    };
}

static SnukValue get(SnukInterpreter *intpret) {
    SnukEnv *value_env = interpreter_lookup(intpret, value_str);
    if (!value_env) return (SnukValue){.type = SNUK_VALUE_UNKOWN};
    if (!(value_env->value.type == SNUK_VALUE_STRING || value_env->value.type == SNUK_VALUE_NULL))
        return (SnukValue){.type = SNUK_VALUE_UNKOWN};

    if (value_env->value.type == SNUK_VALUE_NULL) return snuk_value_copy(value_env->value);

    SnukEnv **envs = snuk_native_get_params(intpret);

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
