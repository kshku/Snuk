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
    SnukStringView name;
    SnukValue value;
} SnukEnv;

typedef struct SnukScope SnukScope;
struct SnukScope {
    SnukEnv **vars; // darray
    SnukScope *parent;
};

typedef struct SnukInterpreter {
    SnukScope *current;
    SnukScope *global;
} SnukInterpreter;

SNUK_INLINE void snuk_interpreter_init(SnukInterpreter *i) {
    SnukScope *scope = (SnukScope *)snuk_alloc(sizeof(SnukScope), alignof(SnukScope));
    *scope = (SnukScope){
        .vars = snuk_darray_create(SnukEnv *),
        .parent = NULL,
    };
    *i = (SnukInterpreter){
        .current = scope,
        .global = scope,
    };
}

SNUK_INLINE void snuk_interpreter_deinit(SnukInterpreter *i) {
    if (!i) return;
    while (i->current) {
        SnukScope *parent = i->current->parent;
        snuk_darray_destroy(i->current->vars);
        snuk_free(i->current);
        i->current = parent;
    }

    *i = (SnukInterpreter){0};
}

SnukValue snuk_interpreter_exec_item(SnukInterpreter *i, SnukItem *item);
SnukValue snuk_interpreter_eval_expr(SnukInterpreter *i, SnukExpr *expr);

void snuk_interpreter_log_value(SnukValue value);
void snuk_interpreter_print_value(SnukValue value);
