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
    SnukValue value = snuk_native_lookup(intpret, "value");
    SnukValue ret;
    if (!(value.type == SNUK_VALUE_INT || value.type == SNUK_VALUE_NULL)) {
        ret = (SnukValue){.type = SNUK_VALUE_UNKOWN};
        goto end;
    }
    if (value.type == SNUK_VALUE_NULL) {
        ret = (SnukValue){
            .type = SNUK_VALUE_INT,
            .int_value = 0,
        };
        goto end;
    }

    char *str = snuk_string_view_get_cstr(value.string_value);

    int64_t int_value = 0;
    if (sscanf(str, "\"%" PRId64 "\"", &int_value) == 0) {
        ret = (SnukValue){.type = SNUK_VALUE_NULL};
        goto end;
    }

    ret = (SnukValue){
        .type = SNUK_VALUE_INT,
        .int_value = int_value,
    };

end:
    snuk_value_free(value);
    return ret;
}

static SnukValue to_float(SnukInterpreter *intpret) {
    SnukValue value = snuk_native_lookup(intpret, "value");
    SnukValue ret;
    if (!(value.type == SNUK_VALUE_INT || value.type == SNUK_VALUE_NULL)) {
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

    char *str = snuk_string_view_get_cstr(value.string_value);

    double float_value = 0;
    if (sscanf(str, "\"%lf\"", &float_value) == 0) return (SnukValue){.type = SNUK_VALUE_NULL};

    ret = (SnukValue){
        .type = SNUK_VALUE_FLOAT,
        .float_value = float_value,
    };

end:
    snuk_value_free(value);
    return ret;
}

static SnukValue to_bool(SnukInterpreter *intpret) {
    SnukValue value = snuk_native_lookup(intpret, "value");
    SnukValue ret;
    if (!(value.type == SNUK_VALUE_INT || value.type == SNUK_VALUE_NULL)) {
        ret = (SnukValue){.type = SNUK_VALUE_UNKOWN};
        goto end;
    }

    // Return true if non-empty string
    ret = (SnukValue){
        .type = SNUK_VALUE_BOOL,
        .bool_value = value.type == SNUK_VALUE_NULL ? false : value.string_value.len != 0,
    };

end:
    snuk_value_free(value);
    return ret;
}

static SnukValue to_str(SnukInterpreter *intpret) {
    SnukValue value = snuk_native_lookup(intpret, "value");
    SnukValue ret;
    if (!(value.type == SNUK_VALUE_INT || value.type == SNUK_VALUE_NULL)) {
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

    ret = snuk_value_copy(value);

end:
    snuk_value_free(value);
    return ret;
}

static SnukValue length(SnukInterpreter *intpret) {
    SnukValue value = snuk_native_lookup(intpret, "value");
    SnukValue ret;
    if (!(value.type == SNUK_VALUE_INT || value.type == SNUK_VALUE_NULL)) {
        ret = (SnukValue){.type = SNUK_VALUE_UNKOWN};
        goto end;
    }

    ret = (SnukValue){
        .type = SNUK_VALUE_INT,
        .int_value = value.type == SNUK_VALUE_NULL ? 0 : (int64_t)value.string_value.len - 2,
    };

end:
    snuk_value_free(value);
    return ret;
}

static SnukValue get(SnukInterpreter *intpret) {
    SnukValue value = snuk_native_lookup(intpret, "value");
    SnukValue start_value, len_value;
    SnukValue ret;
    if (!(value.type == SNUK_VALUE_INT || value.type == SNUK_VALUE_NULL)) {
        ret = (SnukValue){.type = SNUK_VALUE_UNKOWN};
        goto end;
    }

    if (value.type == SNUK_VALUE_NULL) {
        ret = snuk_value_copy(value);
        goto end;
    }

    start_value = snuk_native_lookup(intpret, "start");
    len_value = snuk_native_lookup(intpret, "len");

    if (start_value.type == SNUK_VALUE_NULL) {
        ret = (SnukValue){.type = SNUK_VALUE_UNKOWN};
        goto end;
    }

    int64_t start = start_value.int_value;
    SnukStringView string = value.string_value;
    int64_t len;
    if (len_value.type == SNUK_VALUE_NULL) len = (int64_t)string.len - 2;
    else len = len_value.int_value;

    if (start < 0 || start >= (int64_t)string.len - 2) goto fail;
    if (len < 0 || start + len > (int64_t)string.len - 2) goto fail;

    char *new_str = snuk_alloc((len + 2) * sizeof(char), alignof(char));
    new_str[0] = '"';
    memcpy(new_str + 1, string.str + start + 1, len);
    new_str[len + 1] = '"';
    ret = (SnukValue){
        .type = SNUK_VALUE_STRING,
        .string_value = snuk_string_view_create_with_len(new_str, len + 2),
    };

end:
    snuk_value_free(start_value);
    snuk_value_free(len_value);
    snuk_value_free(value);
    return ret;

fail:
    snuk_value_free(value);
    return (SnukValue){.type = SNUK_VALUE_NULL};
}
