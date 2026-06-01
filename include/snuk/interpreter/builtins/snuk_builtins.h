#pragma once

#include "snuk/darray.h"
#include "snuk/defines.h"
#include "snuk/interpreter/interpreter.h"
#include "snuk/interpreter/interpreter_helper.h"
#include "snuk/interpreter/snuk_scope.h"
#include "snuk/interpreter/snuk_value.h"
#include "snuk/parser/snuk_type.h"
#include "snuk/refcount.h"
#include "snuk/string_view.h"

extern SnukStringView self_str;
extern SnukStringView value_str;
extern SnukType int_type;
extern SnukType float_type;
extern SnukType bool_type;
extern SnukType str_type;

typedef struct BuiltinMember {
    const char *field;
    SnukValue (*build_field)(SnukInterpreter *intpret);
} BuiltinMember;

typedef struct Parameters {
    SnukStringView name;
    SnukType *type;
    SnukValue value;  // can be SNUK_VALUE_UNKOWN
} Parameters;

SnukValue builtin_int_create_type(SnukInterpreter *intpret, bool weak_ref);
SnukValue builtin_float_create_type(SnukInterpreter *intpret, bool weak_ref);
SnukValue builtin_bool_create_type(SnukInterpreter *intpret, bool weak_ref);
SnukValue builtin_str_create_type(SnukInterpreter *intpret, bool weak_ref);

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

SNUK_INLINE SnukValue snuk_builtins_create_type(SnukInterpreter *intpret, SnukValueType type, bool weak_ref) {
    switch (type) {
        case SNUK_VALUE_INT:
            return builtin_int_create_type(intpret, weak_ref);
        case SNUK_VALUE_FLOAT:
            return builtin_float_create_type(intpret, weak_ref);
        case SNUK_VALUE_BOOL:
            return builtin_bool_create_type(intpret, weak_ref);
        case SNUK_VALUE_STRING:
            return builtin_str_create_type(intpret, weak_ref);

        default:
            SNUK_SHOULD_NOT_REACH_HERE;
            break;
    }
    return (SnukValue){.type = SNUK_VALUE_UNKOWN};
}

SNUK_INLINE bool snuk_builtins_create_builtin_types(SnukInterpreter *intpret, bool weak_ref) {
    for (uint64_t i = 0; i < SNUK_ARRAY_LENGTH(builtin_types); ++i) {
        SnukStringView name = snuk_string_view_create(builtin_types[i].type);
        SnukValue type = snuk_builtins_create_type(intpret, builtin_types[i].val_type, weak_ref);
        if (type.type == SNUK_VALUE_UNKOWN) return false;
        if (!snuk_interpreter_create_env(intpret, name, &type_type, type, false)) return false;
        snuk_value_free(type);
    }
    return true;
}

SNUK_INLINE void add_paramters(SnukRefCounter *scope, Parameters *params, uint64_t count) {
    for (uint64_t i = 0; i < count; ++i) {
        SnukEnv *env = snuk_env_create(params[i].name, params[i].type, params[i].value);
        SNUK_ASSERT(env, "failed to create env");
        SNUK_ASSERT(snuk_scope_add_env(scope, env), "failed to add paramter to builtin function");
    }
}

SNUK_INLINE void get_parameters(SnukRefCounter *scope, SnukStringView *names, SnukEnv **envs, uint64_t count) {
    for (uint64_t i = 0; i < count; ++i) {
        envs[i] = snuk_scope_lookup_recursive(scope, names[i]);
        SNUK_ASSERT(envs[i], "failed to get parameter");
    }
}
