#include "builtin_bool.h"

static SnukValue to_int(SnukInterpreter *intpret);
static SnukValue to_float(SnukInterpreter *intpret);
static SnukValue to_bool(SnukInterpreter *intpret);
static SnukValue to_str(SnukInterpreter *intpret);

static SnukValue build_to_int(SnukValue value);
static SnukValue build_to_float(SnukValue value);
static SnukValue build_to_bool(SnukValue value);
static SnukValue build_to_str(SnukValue value);

static BuiltinMember bool_members[] = {
    {.field = "to_int",   .build_field = build_to_int  },
    {.field = "to_float", .build_field = build_to_float},
    {.field = "to_bool",  .build_field = build_to_bool },
    {.field = "to_str",   .build_field = build_to_str  },
};

SnukType bool_type = {
    .type = TYPE_NAMED,
    .name = {.str = "bool", .len = 4}
};

SnukValue builtin_bool_get_member(SnukValue value, SnukStringView field) {
    SNUK_ASSERT(value.type == SNUK_VALUE_BOOL, "something went wrong");
    for (uint64_t i = 0; i < SNUK_ARRAY_LENGTH(bool_members); ++i)
        if (snuk_string_view_equal_cstr(field, bool_members[i].field))
            return bool_members[i].build_field(value);
    return (SnukValue){.type = SNUK_VALUE_UNKOWN};
}

static SnukValue build_to_int(SnukValue value) {
    SnukRefCounter *scope = snuk_scope_create(NULL, false);

    Parameters params[] = {
        {.name = self, .type = &bool_type, value}, // self should be last to add
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
        {.name = self, .type = &bool_type, value}, // self should be last to add
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
        {.name = self, .type = &bool_type, value}, // self should be last to add
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
        {.name = self, .type = &bool_type, value}, // self should be last to add
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

static SnukValue to_int(SnukInterpreter *intpret) {
    SnukEnv *self_env = snuk_scope_lookup(intpret->current, self);
    SNUK_ASSERT(self_env, "something went wrong!");
    return (SnukValue){
        .type = SNUK_VALUE_INT,
        .int_value = (int64_t)(self_env->value.bool_value == true),
    };
}

static SnukValue to_float(SnukInterpreter *intpret) {
    SnukEnv *self_env = snuk_scope_lookup(intpret->current, self);
    SNUK_ASSERT(self_env, "something went wrong!");
    return (SnukValue){
        .type = SNUK_VALUE_FLOAT,
        .float_value = (double)(self_env->value.bool_value == true),
    };
}

static SnukValue to_bool(SnukInterpreter *intpret) {
    SnukEnv *self_env = snuk_scope_lookup(intpret->current, self);
    SNUK_ASSERT(self_env, "something went wrong!");
    return snuk_value_copy(self_env->value);
}

static SnukValue to_str(SnukInterpreter *intpret) {
    SnukEnv *self_env = snuk_scope_lookup(intpret->current, self);
    SNUK_ASSERT(self_env, "something went wrong!");
    SnukStringView str = self_env->value.bool_value ? snuk_string_view_create_with_len("\"true\"", 6)
                                                    : snuk_string_view_create_with_len("\"false\"", 7);
    return (SnukValue){
        .type = SNUK_VALUE_STRING,
        .string_value = str,
    };
}
