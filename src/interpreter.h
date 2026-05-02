#pragma once

#include "defines.h"
#include "darray.h"

#include "parser.h"
#include "string_view.h"

#include "refcount.h"

typedef enum SnukSignal {
    SNUK_SIGNAL_NONE = 0,
    SNUK_SIGNAL_CONTINUE = 1 << 0,
    SNUK_SIGNAL_BREAK = 1 << 1,
    SNUK_SIGNAL_RETURN = 1 << 2,

    SNUK_SIGNAL_ALL = SNUK_SIGNAL_CONTINUE | SNUK_SIGNAL_BREAK | SNUK_SIGNAL_RETURN
} SnukSignal;

typedef struct SnukValue SnukValue;
typedef struct SnukEnv SnukEnv;
typedef struct SnukScope SnukScope;

struct SnukValue {
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
};

struct SnukEnv {
    SnukStringView name;
    SnukValue value;
};

struct SnukScope {
    SnukEnv **vars; // darray
    SnukRefCounter *parent;
};

typedef struct SnukInterpreter {
    SnukRefCounter *current;
    SnukRefCounter *global;
    SnukSignal signal;
} SnukInterpreter;

SNUK_INLINE void snuk_scope_free(void *data, void *ptr) {
    SNUK_UNUSED(data);
    SnukScope *scope = (SnukScope *)ptr;
    snuk_darray_destroy(scope->vars);
    if (scope->parent) snuk_ref_counter_release(scope->parent);
    snuk_free(scope);
}

SNUK_INLINE SnukRefCounter *snuk_scope_create(SnukRefCounter *parent) {
    SnukScope *scope = (SnukScope *)snuk_alloc(sizeof(SnukScope), alignof(SnukScope));
    *scope = (SnukScope){
        .vars = snuk_darray_create(SnukEnv *),
        .parent = snuk_ref_counter_move(&parent),
    };
    return snuk_ref_counter_create(scope, NULL, snuk_scope_free);
}

SNUK_INLINE void snuk_interpreter_init(SnukInterpreter *i) {
    *i = (SnukInterpreter){
        .global = snuk_scope_create(NULL),
        .signal = SNUK_SIGNAL_NONE,
    };
    i->current = snuk_ref_counter_retain(i->global);
}

SNUK_INLINE void snuk_interpreter_deinit(SnukInterpreter *i) {
    if (!i) return;

    // TODO: freeing all scopes
    if (i->current != i->global) snuk_ref_counter_release(i->current);
    snuk_ref_counter_release(i->global);

    *i = (SnukInterpreter){0};
}

SnukValue snuk_interpreter_exec_item(SnukInterpreter *i, SnukItem *item);
SnukValue snuk_interpreter_eval_expr(SnukInterpreter *i, SnukExpr *expr);

void snuk_interpreter_log_value(SnukValue value);
void snuk_interpreter_print_value(SnukValue value);
