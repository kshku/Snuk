#pragma once

#include "../snuk_value.h"
#include "builtin_bool.h"
#include "builtin_float.h"
#include "builtin_int.h"
#include "builtin_null.h"
#include "builtin_str.h"
#include "defines.h"

typedef struct SnukBuiltinTypes {
    const char *type;
    SnukValueType val_type;
} SnukBuiltinTypes;

static SnukBuiltinTypes builtin_types[] = {
    {.type = "int",   .val_type = SNUK_VALUE_INT   },
    {.type = "float", .val_type = SNUK_VALUE_FLOAT },
    {.type = "bool",  .val_type = SNUK_VALUE_BOOL  },
    {.type = "str",   .val_type = SNUK_VALUE_STRING},
};

SNUK_INLINE SnukValueType snuk_builtin_get_value_type(SnukStringView name) {
    for (uint64_t i = 0; i < SNUK_ARRAY_LENGTH(builtin_types); ++i)
        if (snuk_string_view_equal_cstr(name, builtin_types[i].type))
            return builtin_types[i].val_type;
    return SNUK_VALUE_UNKOWN;
}

SNUK_INLINE SnukValue snuk_builtin_get_member(SnukValue value, SnukStringView field) {
    switch (value.type) {
        case SNUK_VALUE_INT:
            return builtin_int_get_member(value, field);
        case SNUK_VALUE_FLOAT:
            return builtin_float_get_member(value, field);
        case SNUK_VALUE_BOOL:
            return builtin_bool_get_member(value, field);
        case SNUK_VALUE_STRING:
            return builtin_str_get_member(value, field);
        case SNUK_VALUE_NULL:
            return builtin_null_get_member(value, field);

        default:
            break;
    }

    return (SnukValue){.type = SNUK_VALUE_UNKOWN};
}
