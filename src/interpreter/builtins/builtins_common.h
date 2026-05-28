#pragma once

#include "../interpreter.h"
#include "../snuk_env.h"
#include "../snuk_scope.h"
#include "../snuk_value.h"
#include "defines.h"
#include "memory.h"
#include "parser/snuk_type.h"
#include "string_view.h"

// NOTE: When getting member functions, add that value as self and self must be last one to be added
// in the scope. (check build_to_int function for reference)

static SnukStringView self = {.str = "self", .len = 4};

extern SnukType int_type;
extern SnukType float_type;
extern SnukType bool_type;
extern SnukType str_type;

typedef struct BuiltinMember {
    const char *field;
    SnukValue (*build_field)(SnukValue value);
} BuiltinMember;

typedef struct Parameters {
    SnukStringView name;
    SnukType *type;
    SnukValue value;  // can be SNUK_VALUE_UNKOWN
} Parameters;

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
