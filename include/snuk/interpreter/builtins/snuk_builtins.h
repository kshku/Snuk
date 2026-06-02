#pragma once

#include "snuk/interpreter/snuk_value.h"
#include "snuk/parser/snuk_type.h"
#include "snuk/string_view.h"

extern SnukStringView self_str;
extern SnukStringView value_str;
extern SnukType int_type;
extern SnukType float_type;
extern SnukType bool_type;
extern SnukType str_type;

SnukValue builtin_null_get_member(SnukInterpreter *intpret, SnukStringView field);

static struct {
    const char *type;
    SnukValueType val_type;
} builtin_types[] = {
    {.type = "int",   .val_type = SNUK_VALUE_INT   },
    {.type = "float", .val_type = SNUK_VALUE_FLOAT },
    {.type = "bool",  .val_type = SNUK_VALUE_BOOL  },
    {.type = "str",   .val_type = SNUK_VALUE_STRING},
};

SNUK_INLINE SnukValueType snuk_builtins_get_value_type(SnukStringView name) {
    for (uint64_t i = 0; i < SNUK_ARRAY_LENGTH(builtin_types); ++i)
        if (snuk_string_view_equal_cstr(name, builtin_types[i].type))
            return builtin_types[i].val_type;
    return SNUK_VALUE_UNKOWN;
}

SnukValue snuk_builtins_create_type(SnukInterpreter *intpret, SnukValueType type, bool weak_ref);

bool snuk_builtins_create_builtin_types(SnukInterpreter *intpret, bool weak_ref);

void snuk_builtins_init(SnukInterpreter *intpret);
void snuk_builtins_deinit(SnukInterpreter *intpret);
