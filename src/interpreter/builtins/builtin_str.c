#include "builtin_str.h"

#include <stdio.h>

static SnukValue to_int(SnukInterpreter *intpret);
static SnukValue to_float(SnukInterpreter *intpret);
static SnukValue to_bool(SnukInterpreter *intpret);
static SnukValue to_str(SnukInterpreter *intpret);
static SnukValue length(SnukInterpreter *intpret);
static SnukValue get(SnukInterpreter *intpret);

static SnukValue build_to_int(SnukValue value);
static SnukValue build_to_float(SnukValue value);
static SnukValue build_to_bool(SnukValue value);
static SnukValue build_to_str(SnukValue value);
static SnukValue build_length(SnukValue value);
static SnukValue build_get(SnukValue value);

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

SnukValue builtin_str_get_member(SnukValue value, SnukStringView field) {
    SNUK_ASSERT(value.type == SNUK_VALUE_STRING, "something went wrong");
    for (uint64_t i = 0; i < SNUK_ARRAY_LENGTH(str_members); ++i)
        if (snuk_string_view_equal_cstr(field, str_members[i].field))
            return str_members[i].build_field(value);
    return (SnukValue){.type = SNUK_VALUE_UNKOWN};
}

static SnukValue build_to_int(SnukValue value) {
    SnukRefCounter *scope = snuk_scope_create(NULL, false);

    Parameters params[] = {
        {.name = self, .type = &str_type, value}, // self should be last to add
    };

    add_paramters(scope, params, SNUK_ARRAY_LENGTH(params));

    return (SnukValue){
        .type = SNUK_VALUE_FN_BUILTIN,
        .builtin_fn = {
            .closure = scope,
            .fn = to_int,
        },
    };
}

static SnukValue build_to_float(SnukValue value) {
    SnukRefCounter *scope = snuk_scope_create(NULL, false);

    Parameters params[] = {
        {.name = self, .type = &str_type, value}, // self should be last to add
    };

    add_paramters(scope, params, SNUK_ARRAY_LENGTH(params));

    return (SnukValue){
        .type = SNUK_VALUE_FN_BUILTIN,
        .builtin_fn = {
            .closure = scope,
            .fn = to_float,
        },
    };
}

static SnukValue build_to_bool(SnukValue value) {
    SnukRefCounter *scope = snuk_scope_create(NULL, false);

    Parameters params[] = {
        {.name = self, .type = &str_type, value}, // self should be last to add
    };

    add_paramters(scope, params, SNUK_ARRAY_LENGTH(params));

    return (SnukValue){
        .type = SNUK_VALUE_FN_BUILTIN,
        .builtin_fn = {
            .closure = scope,
            .fn = to_bool,
        },
    };
}

static SnukValue build_to_str(SnukValue value) {
    SnukRefCounter *scope = snuk_scope_create(NULL, false);

    Parameters params[] = {
        {.name = self, .type = &str_type, value}, // self should be last to add
    };

    add_paramters(scope, params, SNUK_ARRAY_LENGTH(params));

    return (SnukValue){
        .type = SNUK_VALUE_FN_BUILTIN,
        .builtin_fn = {
            .closure = scope,
            .fn = to_str,
        },
    };
}

static SnukValue build_length(SnukValue value) {
    SnukRefCounter *scope = snuk_scope_create(NULL, false);

    Parameters params[] = {
        {.name = self, .type = &str_type, value}, // self should be last to add
    };

    add_paramters(scope, params, SNUK_ARRAY_LENGTH(params));

    return (SnukValue){
        .type = SNUK_VALUE_FN_BUILTIN,
        .builtin_fn = {
            .closure = scope,
            .fn = length,
        },
    };
}

static SnukValue build_get(SnukValue value) {
    SnukRefCounter *scope = snuk_scope_create(NULL, false);
    Parameters params[] = {
        {
         .name = snuk_string_view_create_with_len("start", 5),
         .type = &int_type,
         .value = (SnukValue){.type = SNUK_VALUE_INT, .int_value = 0},
         },
        {
         .name = snuk_string_view_create_with_len("len", 3),
         .type = &int_type,
         .value = (SnukValue){.type = SNUK_VALUE_NULL},
         },
        {
         .name = self,
         .type = &str_type,
         .value = value,
         }, // self should be last to add
    };

    add_paramters(scope, params, SNUK_ARRAY_LENGTH(params));

    return (SnukValue){
        .type = SNUK_VALUE_FN_BUILTIN,
        .builtin_fn = {
            .closure = scope,
            .fn = get,
        },
    };
}

static SnukValue to_int(SnukInterpreter *intpret) {
    SnukEnv *self_env = snuk_scope_lookup(intpret->current, self);
    SNUK_ASSERT(self_env, "something went wrong!");
    char *str = snuk_string_view_get_cstr(self_env->value.string_value);

    int64_t int_value = 0;
    if (sscanf(str, "\"%" PRId64 "\"", &int_value) == 0)
        return (SnukValue){.type = SNUK_VALUE_NULL};

    return (SnukValue){
        .type = SNUK_VALUE_INT,
        .int_value = int_value,
    };
}

static SnukValue to_float(SnukInterpreter *intpret) {
    SnukEnv *self_env = snuk_scope_lookup(intpret->current, self);
    SNUK_ASSERT(self_env, "something went wrong!");
    char *str = snuk_string_view_get_cstr(self_env->value.string_value);

    double float_value = 0;
    if (sscanf(str, "\"%lf\"", &float_value) == 0) return (SnukValue){.type = SNUK_VALUE_NULL};

    return (SnukValue){
        .type = SNUK_VALUE_FLOAT,
        .float_value = float_value,
    };
}

static SnukValue to_bool(SnukInterpreter *intpret) {
    SnukEnv *self_env = snuk_scope_lookup(intpret->current, self);
    SNUK_ASSERT(self_env, "something went wrong!");
    // Return true if non-empty string
    return (SnukValue){
        .type = SNUK_VALUE_BOOL,
        .bool_value = self_env->value.string_value.len != 0,
    };
}

static SnukValue to_str(SnukInterpreter *intpret) {
    SnukEnv *self_env = snuk_scope_lookup(intpret->current, self);
    SNUK_ASSERT(self_env, "something went wrong!");
    return snuk_value_copy(self_env->value);
}

static SnukValue length(SnukInterpreter *intpret) {
    SnukEnv *self_env = snuk_scope_lookup(intpret->current, self);
    SNUK_ASSERT(self_env, "something went wrong!");
    return (SnukValue){
        .type = SNUK_VALUE_INT,
        .int_value = (int64_t)self_env->value.string_value.len - 2,
    };
}

static SnukValue get(SnukInterpreter *intpret) {
    SnukEnv *self_env = snuk_scope_lookup(intpret->current, self);
    SNUK_ASSERT(self_env, "something went wrong!");
    SnukEnv *envs[2];
    SnukStringView names[] = {
        snuk_string_view_create_with_len("start", 5),
        snuk_string_view_create_with_len("len", 3),
        self,
    };
    get_parameters(intpret->current, names, envs, 2);

    int64_t start = envs[0]->value.int_value;
    SnukStringView string = self_env->value.string_value;
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
