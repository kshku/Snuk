#pragma once

#include "defines.h"
#include "darray.h"

#include "parser.h"

typedef struct Value {
    enum {
        VALUE_UNKOWN,
        VALUE_INT,
        VALUE_FLOAT,
        VALUE_BOOL,
        VALUE_STRING,
        VALUE_NULL,
    } type;

    union {
        int64_t int_value;
        double float_value;
        bool bool_value;
        struct { 
            const char *string;
            uint64_t length;
        } string_value;
    };
} Value;

typedef struct SnukIdentifier{
    const char *name;
    uint64_t length;
} SnukIdentifier;

typedef struct Env {
    SnukIdentifier identifier;
    Value value;
} Env;

typedef struct SnukInterpreter {
    Env **envs; // darray
} SnukInterpreter;

SNUK_INLINE void snuk_interpreter_init(SnukInterpreter *i) {
    *i = (SnukInterpreter){
        .envs = snuk_darray_create_with_capacity(128, Env *),
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

void snuk_interpreter_exec_stmt(SnukInterpreter *i, SnukStmt *stmt);
Value snuk_interpreter_eval_expr(SnukInterpreter *i, SnukExpr *expr);

void snuk_interpreter_print_value(Value value);
