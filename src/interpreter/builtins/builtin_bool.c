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

SnukTypeMember bool_members[] = {
    {.name = "value",    .type = &any_type,      .value = {.type = SNUK_VALUE_NULL}, .is_const = false},
    {.name = "to_int",   .type = &to_int_type,   .build_value = build_to_int,        .is_const = false},
    {.name = "to_float", .type = &to_float_type, .build_value = build_to_float,      .is_const = false},
    {.name = "to_bool",  .type = &to_bool_type,  .build_value = build_to_bool,       .is_const = false},
    {.name = "to_str",   .type = &to_str_type,   .build_value = build_to_str,        .is_const = false},
};

SnukValue builtin_bool_create_type(SnukInterpreter *intpret, bool weak_ref) {
    return snuk_native_create_type(intpret, bool_members, SNUK_ARRAY_LENGTH(bool_members), weak_ref);
}

SnukType bool_type = {
    .type = TYPE_NAMED,
    .name = {.str = "bool", .len = 4}
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

static SnukValue to_int(SnukInterpreter *intpret) {
    SnukValue value = snuk_native_lookup(intpret, "value");
    SnukValue ret;
    if (!(value.type == SNUK_VALUE_BOOL || value.type == SNUK_VALUE_NULL)) {
        ret = (SnukValue){.type = SNUK_VALUE_UNKOWN};
        goto end;
    }

    ret = (SnukValue){
        .type = SNUK_VALUE_INT,
        .int_value = value.type == SNUK_VALUE_NULL ? 0 : (int64_t)(value.bool_value == true),
    };

end:
    snuk_value_free(value);
    return ret;
}

static SnukValue to_float(SnukInterpreter *intpret) {
    SnukValue value = snuk_native_lookup(intpret, "value");
    SnukValue ret;
    if (!(value.type == SNUK_VALUE_BOOL || value.type == SNUK_VALUE_NULL)) {
        ret = (SnukValue){.type = SNUK_VALUE_UNKOWN};
        goto end;
    }

    ret = (SnukValue){
        .type = SNUK_VALUE_FLOAT,
        .float_value = value.type == SNUK_VALUE_NULL ? 0.0 : (double)(value.bool_value == true),
    };

end:
    snuk_value_free(value);
    return ret;
}

static SnukValue to_bool(SnukInterpreter *intpret) {
    SnukValue value = snuk_native_lookup(intpret, "value");
    SnukValue ret;
    if (!(value.type == SNUK_VALUE_BOOL || value.type == SNUK_VALUE_NULL)) {
        ret = (SnukValue){.type = SNUK_VALUE_UNKOWN};
        goto end;
    }

    if (value.type == SNUK_VALUE_NULL) {
        ret = (SnukValue){
            .type = SNUK_VALUE_BOOL,
            .bool_value = false,
        };
        goto end;
    }

    ret = snuk_value_copy(value);

end:
    snuk_value_free(value);
    return ret;
}

static SnukValue to_str(SnukInterpreter *intpret) {
    SnukValue value = snuk_native_lookup(intpret, "value");
    SnukValue ret;
    if (!(value.type == SNUK_VALUE_BOOL || value.type == SNUK_VALUE_NULL)) {
        ret = (SnukValue){.type = SNUK_VALUE_UNKOWN};
        goto end;
    }

    SnukStringView str;
    if (value.type == SNUK_VALUE_NULL) snuk_string_view_create_with_len("\"null\"", 6);
    else
        str = value.bool_value ? snuk_string_view_create_with_len("\"true\"", 6)
                               : snuk_string_view_create_with_len("\"false\"", 7);
    ret = (SnukValue){
        .type = SNUK_VALUE_STRING,
        .string_value = str,
    };

end:
    snuk_value_free(value);
    return ret;
}
