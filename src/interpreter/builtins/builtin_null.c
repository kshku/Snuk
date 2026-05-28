#include "builtin_null.h"

static SnukValue to_int(SnukInterpreter *intpret);
static SnukValue to_float(SnukInterpreter *intpret);
static SnukValue to_bool(SnukInterpreter *intpret);
static SnukValue to_str(SnukInterpreter *intpret);

static SnukValue build_to_int(SnukValue value);
static SnukValue build_to_float(SnukValue value);
static SnukValue build_to_bool(SnukValue value);
static SnukValue build_to_str(SnukValue value);

static BuiltinMember null_members[] = {
    {.field = "to_int",   .build_field = build_to_int  },
    {.field = "to_float", .build_field = build_to_float},
    {.field = "to_bool",  .build_field = build_to_bool },
    {.field = "to_str",   .build_field = build_to_str  },
};

SnukValue builtin_null_get_member(SnukValue value, SnukStringView field) {
    SNUK_ASSERT(value.type == SNUK_VALUE_NULL, "something went wrong");
    for (uint64_t i = 0; i < SNUK_ARRAY_LENGTH(null_members); ++i)
        if (snuk_string_view_equal_cstr(field, null_members[i].field))
            return null_members[i].build_field(value);
    return (SnukValue){.type = SNUK_VALUE_UNKOWN};
}

static SnukValue build_to_int(SnukValue value) {
    SNUK_UNUSED(value);
    SnukRefCounter *scope = snuk_scope_create(NULL, false);
    return (SnukValue){
        .type = SNUK_VALUE_FN_BUILTIN,
        .builtin_fn = {
            .closure = scope,
            .fn = to_int,
        },
    };
}

static SnukValue build_to_float(SnukValue value) {
    SNUK_UNUSED(value);
    SnukRefCounter *scope = snuk_scope_create(NULL, false);
    return (SnukValue){
        .type = SNUK_VALUE_FN_BUILTIN,
        .builtin_fn = {
            .closure = scope,
            .fn = to_float,
        },
    };
}

static SnukValue build_to_bool(SnukValue value) {
    SNUK_UNUSED(value);
    SnukRefCounter *scope = snuk_scope_create(NULL, false);
    return (SnukValue){
        .type = SNUK_VALUE_FN_BUILTIN,
        .builtin_fn = {
            .closure = scope,
            .fn = to_bool,
        },
    };
}

static SnukValue build_to_str(SnukValue value) {
    SNUK_UNUSED(value);
    SnukRefCounter *scope = snuk_scope_create(NULL, false);
    return (SnukValue){
        .type = SNUK_VALUE_FN_BUILTIN,
        .builtin_fn = {
            .closure = scope,
            .fn = to_str,
        },
    };
}

static SnukValue to_int(SnukInterpreter *intpret) {
    SNUK_UNUSED(intpret);
    return (SnukValue){
        .type = SNUK_VALUE_INT,
        .int_value = 0,
    };
}

static SnukValue to_float(SnukInterpreter *intpret) {
    SNUK_UNUSED(intpret);
    return (SnukValue){
        .type = SNUK_VALUE_FLOAT,
        .float_value = 0.0,
    };
}

static SnukValue to_bool(SnukInterpreter *intpret) {
    SNUK_UNUSED(intpret);
    return (SnukValue){
        .type = SNUK_VALUE_BOOL,
        .bool_value = false,
    };
}

static SnukValue to_str(SnukInterpreter *intpret) {
    SNUK_UNUSED(intpret);
    return (SnukValue){
        .type = SNUK_VALUE_STRING,
        .string_value = snuk_string_view_create_with_len("null", 4),
    };
}
