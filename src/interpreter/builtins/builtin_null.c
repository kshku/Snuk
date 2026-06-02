#include "snuk/interpreter/builtins/snuk_builtins.h"

static SnukValue to_int(SnukInterpreter *intpret);
static SnukValue to_float(SnukInterpreter *intpret);
static SnukValue to_bool(SnukInterpreter *intpret);
static SnukValue to_str(SnukInterpreter *intpret);

static SnukValue build_to_int(SnukInterpreter *intpret);
static SnukValue build_to_float(SnukInterpreter *intpret);
static SnukValue build_to_bool(SnukInterpreter *intpret);
static SnukValue build_to_str(SnukInterpreter *intpret);

static BuiltinMember null_members[] = {
    {.field = "to_int",   .build_field = build_to_int  },
    {.field = "to_float", .build_field = build_to_float},
    {.field = "to_bool",  .build_field = build_to_bool },
    {.field = "to_str",   .build_field = build_to_str  },
};

static SnukType to_int_type = {
    .type = TYPE_FN,
    .fn = {
        .return_type = &int_type,
    },
};

static SnukType to_float_type = {
    .type = TYPE_FN,
    .fn = {
        .return_type = &float_type,
    },
};

static SnukType to_bool_type = {
    .type = TYPE_FN,
    .fn = {
        .return_type = &bool_type,
    },
};

static SnukType to_str_type = {
    .type = TYPE_FN,
    .fn = {
        .return_type = &str_type,
    },
};

static SnukRefCounter *empty_scope;

void builtin_null_init(SnukInterpreter *intpret) {
    if (!to_int_type.fn.param_types)
        to_int_type.fn.param_types = snuk_darray_create(SnukType *, &intpret->allocator);
    if (!to_float_type.fn.param_types)
        to_float_type.fn.param_types = snuk_darray_create(SnukType *, &intpret->allocator);
    if (!to_bool_type.fn.param_types)
        to_bool_type.fn.param_types = snuk_darray_create(SnukType *, &intpret->allocator);
    if (!to_str_type.fn.param_types)
        to_str_type.fn.param_types = snuk_darray_create(SnukType *, &intpret->allocator);
    if (!empty_scope) empty_scope = snuk_scope_create(NULL, false);
}

void builtin_null_deinit(SnukInterpreter *intpret) {
    SNUK_UNUSED(intpret);
    snuk_ref_counter_release(&empty_scope);
}

SnukValue builtin_null_get_member(SnukInterpreter *intpret, SnukStringView field) {
    for (uint64_t i = 0; i < SNUK_ARRAY_LENGTH(null_members); ++i)
        if (snuk_string_view_equal_cstr(field, null_members[i].field))
            return null_members[i].build_field(intpret);
    return (SnukValue){.type = SNUK_VALUE_UNKOWN};
}

static SnukValue build_to_int(SnukInterpreter *intpret) {
    SNUK_UNUSED(intpret);
    return (SnukValue){
        .type = SNUK_VALUE_FN_BUILTIN,
        .builtin_fn = {
            .closure = snuk_ref_counter_retain(empty_scope),
            .fn = to_int,
            .type = &to_int_type,
        },
    };
}

static SnukValue build_to_float(SnukInterpreter *intpret) {
    SNUK_UNUSED(intpret);
    return (SnukValue){
        .type = SNUK_VALUE_FN_BUILTIN,
        .builtin_fn = {
            .closure = snuk_ref_counter_retain(empty_scope),
            .fn = to_float,
            .type = &to_float_type,
        },
    };
}

static SnukValue build_to_bool(SnukInterpreter *intpret) {
    SNUK_UNUSED(intpret);
    return (SnukValue){
        .type = SNUK_VALUE_FN_BUILTIN,
        .builtin_fn = {
            .closure = snuk_ref_counter_retain(empty_scope),
            .fn = to_bool,
            .type = &to_bool_type,
        },
    };
}

static SnukValue build_to_str(SnukInterpreter *intpret) {
    SNUK_UNUSED(intpret);
    return (SnukValue){
        .type = SNUK_VALUE_FN_BUILTIN,
        .builtin_fn = {
            .closure = snuk_ref_counter_retain(empty_scope),
            .fn = to_str,
            .type = &to_str_type,
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
