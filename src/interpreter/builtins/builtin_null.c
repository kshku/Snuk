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

SnukValue builtin_null_get_member(SnukInterpreter *intpret, SnukStringView field) {
    for (uint64_t i = 0; i < SNUK_ARRAY_LENGTH(null_members); ++i)
        if (snuk_string_view_equal_cstr(field, null_members[i].field))
            return null_members[i].build_field(intpret);
    return (SnukValue){.type = SNUK_VALUE_UNKOWN};
}

static SnukValue build_to_int(SnukInterpreter *intpret) {
    SnukRefCounter *scope = snuk_scope_create(NULL, false);

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
    SnukRefCounter *scope = snuk_scope_create(NULL, false);

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
    SnukRefCounter *scope = snuk_scope_create(NULL, false);

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
    SnukRefCounter *scope = snuk_scope_create(NULL, false);

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
