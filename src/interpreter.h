#pragma once

#include "defines.h"
#include "darray.h"

#include "parser.h"
#include "string_view.h"

typedef struct SnukValue {
    enum {
        SNUK_VALUE_UNKOWN,
        SNUK_VALUE_INT,
        SNUK_VALUE_FLOAT,
        SNUK_VALUE_BOOL,
        SNUK_VALUE_STRING,
        SNUK_VALUE_NULL,
        SNUK_VALUE_FN,
        SNUK_VALUE_TYPE,

        SNUK_VALUE_MAX
    } type;

    union {
        int64_t int_value;
        double float_value;
        bool bool_value;
        SnukStringView string_value;
        // TODO: function and type
    };
} SnukValue;

typedef struct SnukEnv {
    SnukStringView identifier;
    SnukValue value;
} SnukEnv;

typedef struct SnukInterpreter {
    SnukEnv **envs; // darray
} SnukInterpreter;

SNUK_INLINE void snuk_interpreter_init(SnukInterpreter *i) {
    *i = (SnukInterpreter){
        .envs = snuk_darray_create_with_capacity(128, SnukEnv *),
    };
}

SNUK_INLINE void snuk_interpreter_deinit(SnukInterpreter *i) {
    if (!i) return;
    if (!i->envs) return;
    uint64_t count = snuk_darray_get_length(i->envs);
    for (uint64_t j = 0; j < count; ++j)
        if (i->envs[j]) snuk_darray_destroy(i->envs[j]);

    snuk_darray_destroy(i->envs);
    *i = (SnukInterpreter){0};
}

SnukValue snuk_interpreter_exec_item(SnukInterpreter *i, SnukItem *item);
SnukValue snuk_interpreter_eval_expr(SnukInterpreter *i, SnukExpr *expr);

void snuk_interpreter_print_value(SnukValue value);
