#include "builtin_common.h"
#include "snuk/interpreter/builtins/snuk_builtins.h"

static SnukValue to_int(SnukInterpreter *intpret);
static SnukValue to_float(SnukInterpreter *intpret);
static SnukValue to_bool(SnukInterpreter *intpret);
static SnukValue to_str(SnukInterpreter *intpret);

static SnukValue build_to_int(SnukInterpreter *intpret, bool weak_ref);
static SnukValue build_to_float(SnukInterpreter *intpret, bool weak_ref);
static SnukValue build_to_bool(SnukInterpreter *intpret, bool weak_ref);
static SnukValue build_to_str(SnukInterpreter *intpret, bool weak_ref);

static SnukTypeMember null_members[] = {
    {.name = "to_int",   .type = &to_int_type,   .build_value = build_to_int,   .is_const = false},
    {.name = "to_float", .type = &to_float_type, .build_value = build_to_float, .is_const = false},
    {.name = "to_bool",  .type = &to_bool_type,  .build_value = build_to_bool,  .is_const = false},
    {.name = "to_str",   .type = &to_str_type,   .build_value = build_to_str,   .is_const = false},
};

SnukValue builtin_null_get_member(SnukInterpreter *intpret, SnukStringView field) {
    for (uint64_t i = 0; i < SNUK_ARRAY_LENGTH(null_members); ++i)
        if (snuk_string_view_equal_cstr(field, null_members[i].name))
            return null_members[i].build_value(intpret, true);
    return (SnukValue){.type = SNUK_VALUE_UNKOWN};
}

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
