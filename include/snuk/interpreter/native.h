#pragma once

#include "interpreter.h"
#include "snuk/defines.h"
#include "snuk/parser/snuk_type.h"
#include "snuk/string_view.h"
#include "snuk_scope.h"
#include "snuk_value.h"

typedef SnukValue (*build_value_t)(SnukInterpreter *intpret, bool weak_ref);

typedef struct SnukParameter {
    const char *name;
    build_value_t build_value;  // can be SNUK_VALUE_UNKOWN for param without default value
    SnukValue value;  // if build_value is null this one is used
} SnukParameter;

typedef SnukValue (*native_function_t)(SnukInterpreter *intpret);

typedef struct SnukTypeMember {
    const char *name;
    SnukType *type;
    bool is_const;
    build_value_t build_value;  // Shouldn't be SNUK_VALUE_UNKOWN
    SnukValue value;  // if build_value is null this one is used
} SnukTypeMember;

SNUK_INLINE bool snuk_native_add_value(
    SnukInterpreter *intpret, const char *name, SnukType *type, SnukValue value, bool is_const) {
    SnukStringView name_sv = snuk_string_view_create(name);
    if (!snuk_interpreter_create_env(intpret, name_sv, type, value, is_const)) return false;
    return true;
}

SNUK_API SnukValue snuk_native_lookup(SnukInterpreter *intpret, const char *name);

SNUK_API SnukValue snuk_native_get_member(SnukInterpreter *intpret, SnukValue type_or_inst, const char *name);

SNUK_API SnukValue snuk_native_create_type(
    SnukInterpreter *intpret, SnukTypeMember *members, uint64_t count, bool weak_ref);

SNUK_API SnukValue snuk_native_create_fn(SnukInterpreter *intpret, SnukParameter *params, uint64_t count,
                                         SnukType *fn_type, native_function_t fn, bool weak_ref);

SNUK_API SnukValue snuk_native_create_inst(
    SnukInterpreter *intpret, SnukType *type, SnukTypeMember *members, uint64_t count, bool weak_ref);

SNUK_API SnukValue snuk_native_create_int(SnukInterpreter *intpret, int64_t value, bool weak_ref);

SNUK_API SnukValue snuk_native_create_float(SnukInterpreter *intpret, double value, bool weak_ref);

SNUK_API SnukValue snuk_native_create_bool(SnukInterpreter *intpret, bool value, bool weak_ref);

SNUK_API SnukValue snuk_native_create_string(SnukInterpreter *intpret, const char *str, bool weak_ref);

SNUK_API SnukValue snuk_native_create_null(SnukInterpreter *intpret, bool weak_ref);

SNUK_API SnukValue snuk_native_create_interface(SnukInterpreter *intpret, SnukType *interface, bool weak_ref);
