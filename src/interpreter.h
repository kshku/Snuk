#pragma once

#include "defines.h"
#include "darray.h"

#include "parser.h"
#include "string_view.h"

#include "refcount.h"

/**
 * @brief Control-flow signal flags raised during item or expression evaluation.
 *
 * The flag values are bit positions so they can be combined into capture and
 * propagate masks (see SNUK_SIGNAL_ALL). Loops and blocks pass these masks to
 * execute_block_expr to decide which signals stop the current frame and which
 * bubble up to an enclosing frame.
 */
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

/**
 * @brief Runtime value produced by expression evaluation.
 *
 * The type tag selects which union member is meaningful: int_value for
 * integers, float_value for floats, bool_value for booleans, string_value for
 * string views, and fn_value for function values. SNUK_VALUE_NULL,
 * SNUK_VALUE_UNKOWN, and SNUK_VALUE_TYPE carry no payload today.
 *
 * @note A function value holds a refcounted reference to its closure scope,
 * which keeps the captured bindings alive for as long as the function value
 * is reachable.
 */
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
        struct {
            SnukRefCounter *closure;
            SnukExpr *body;
            SnukParam **params;
            SnukType *return_type;
        } fn_value;
        // TODO: type
    };
};

/**
 * @brief Single name-to-value binding inside a scope.
 */
struct SnukEnv {
    SnukStringView name;
    SnukValue value;
};

/**
 * @brief Lexical scope holding variable bindings and a parent reference.
 *
 * vars is a darray of owned SnukEnv pointers. parent is a refcounted handle
 * to the enclosing scope, or NULL for the global scope.
 */
struct SnukScope {
    SnukEnv **vars; // darray
    SnukRefCounter *parent;
};

/**
 * @brief Mutable interpreter state shared across exec and eval calls.
 *
 * current is the active scope stack head; it changes as blocks, functions,
 * and for loops push and pop scopes. global is retained for the lifetime of
 * the interpreter so identifiers can fall through to the root. signal carries
 * the most recent control-flow signal raised during evaluation.
 */
typedef struct SnukInterpreter {
    SnukRefCounter *current;
    SnukRefCounter *global;
    SnukSignal signal;
} SnukInterpreter;

/**
 * @brief Initialize an interpreter with a fresh global scope.
 *
 * Creates the global scope, sets the current scope to point at it, and
 * clears any pending signal.
 *
 * @param intpret Interpreter state to initialize.
 */
void snuk_interpreter_init(SnukInterpreter *intpret);

/**
 * @brief Release the interpreter's scopes and reset the state.
 *
 * Releases the current scope (when distinct from global) and the global
 * scope, then zeroes the struct. Safe to call with a NULL pointer.
 *
 * @param intpret Interpreter state to release, or NULL.
 */
void snuk_interpreter_deinit(SnukInterpreter *intpret);

/**
 * @brief Execute a top-level parsed item.
 *
 * Evaluates the item, applies any side effects (declaration, print,
 * control-flow signal), and returns the resulting runtime value. Return,
 * break, and continue items raise the matching signal on the interpreter
 * before returning.
 *
 * @param i Interpreter state.
 * @param item Parsed item to execute.
 *
 * @return The value produced by the item, or a NULL/UNKOWN placeholder when
 * the item has no meaningful result.
 */
SnukValue snuk_interpreter_exec_item(SnukInterpreter *intpret, SnukItem *item);

/**
 * @brief Evaluate an expression and return its runtime value.
 *
 * Recursively evaluates literals, identifiers, unary and binary expressions,
 * assignments, blocks, conditionals, while/do-while/for loops, function
 * expressions, and function calls.
 *
 * @param i Interpreter state.
 * @param expr Expression to evaluate.
 *
 * @return The value the expression evaluates to.
 */
SnukValue snuk_interpreter_eval_expr(SnukInterpreter *intpret, SnukExpr *expr);

/**
 * @brief Log a runtime value to the trace logger for debugging.
 *
 * @param value Value to log.
 */
void snuk_interpreter_log_value(SnukValue value);

/**
 * @brief Print a runtime value to standard output.
 *
 * @param value Value to print.
 */
void snuk_interpreter_print_value(SnukValue value);
