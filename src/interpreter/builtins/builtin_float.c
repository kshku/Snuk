#include "builtin_common.h"
#include "snuk/interpreter/builtins/snuk_builtins.h"

#include <stdio.h>

static SnukValue to_int(SnukInterpreter *intpret);
static SnukValue to_float(SnukInterpreter *intpret);
static SnukValue to_bool(SnukInterpreter *intpret);
static SnukValue to_str(SnukInterpreter *intpret);

static SnukValue build_to_int(SnukInterpreter *intpret, bool weak_ref);
static SnukValue build_to_float(SnukInterpreter *intpret, bool weak_ref);
static SnukValue build_to_bool(SnukInterpreter *intpret, bool weak_ref);
static SnukValue build_to_str(SnukInterpreter *intpret, bool weak_ref);

SnukTypeMember float_members[] = {
    {.name = "value",    .type = &any_type,      .value = {.type = SNUK_VALUE_NULL}, .is_const = false},
    {.name = "to_int",   .type = &to_int_type,   .build_value = build_to_int,        .is_const = false},
    {.name = "to_float", .type = &to_float_type, .build_value = build_to_float,      .is_const = false},
    {.name = "to_bool",  .type = &to_bool_type,  .build_value = build_to_bool,       .is_const = false},
    {.name = "to_str",   .type = &to_str_type,   .build_value = build_to_str,        .is_const = false},
};

SnukValue builtin_float_create_type(SnukInterpreter *intpret, bool weak_ref) {
    return snuk_native_create_type(intpret, float_members, SNUK_ARRAY_LENGTH(float_members), weak_ref);
}

SnukType float_type = {
    .type = TYPE_NAMED,
    .name = {.str = "float", .len = 5}
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
    if (!(value.type == SNUK_VALUE_FLOAT || value.type == SNUK_VALUE_NULL)) {
        ret = (SnukValue){.type = SNUK_VALUE_UNKOWN};
        goto end;
    }

    ret = (SnukValue){
        .type = SNUK_VALUE_INT,
        .int_value = value.type == SNUK_VALUE_NULL ? 0 : (int64_t)value.float_value,
    };

end:
    snuk_value_free(value);
    return ret;
}

static SnukValue to_float(SnukInterpreter *intpret) {
    SnukValue value = snuk_native_lookup(intpret, "value");
    SnukValue ret;
    if (!(value.type == SNUK_VALUE_FLOAT || value.type == SNUK_VALUE_NULL)) {
        ret = (SnukValue){.type = SNUK_VALUE_UNKOWN};
        goto end;
    }

    if (value.type == SNUK_VALUE_NULL) {
        ret = (SnukValue){
            .type = SNUK_VALUE_FLOAT,
            .float_value = 0.0,
        };
        goto end;
    }

    ret = snuk_value_copy(value);

end:
    snuk_value_free(value);
    return ret;
}

static SnukValue to_bool(SnukInterpreter *intpret) {
    SnukValue value = snuk_native_lookup(intpret, "value");
    SnukValue ret;
    if (!(value.type == SNUK_VALUE_FLOAT || value.type == SNUK_VALUE_NULL)) {
        ret = (SnukValue){.type = SNUK_VALUE_UNKOWN};
        goto end;
    }

    ret = (SnukValue){
        .type = SNUK_VALUE_BOOL,
        .bool_value = value.type == SNUK_VALUE_NULL ? false : (bool)value.float_value,
    };

end:
    snuk_value_free(value);
    return ret;
}

static SnukValue to_str(SnukInterpreter *intpret) {
    SnukValue value = snuk_native_lookup(intpret, "value");
    SnukValue ret;
    if (!(value.type == SNUK_VALUE_FLOAT || value.type == SNUK_VALUE_NULL)) {
        ret = (SnukValue){.type = SNUK_VALUE_UNKOWN};
        goto end;
    }

    if (value.type == SNUK_VALUE_NULL) {
        ret = (SnukValue){
            .type = SNUK_VALUE_STRING,
            .string_value = snuk_string_view_create_with_len("\"null\"", 6),
        };
        goto end;
    }

    char *buf = (char *)snuk_alloc(25 * sizeof(char), alignof(char));
    uint64_t len = 0;
    len = snprintf(buf, 25, "\"%lf\"", value.float_value);
    buf[len] = 0;
    ret = (SnukValue){
        .type = SNUK_VALUE_STRING,
        .string_value = snuk_string_view_create_with_len(buf, len),
    };

end:
    snuk_value_free(value);
    return ret;
}
