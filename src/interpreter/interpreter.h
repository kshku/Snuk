#pragma once

#include "defines.h"
#include "parser/snuk_expr.h"
#include "parser/snuk_item.h"
#include "refcount.h"
#include "snuk_env.h"
#include "snuk_signal.h"
#include "snuk_value.h"
#include "string_view.h"

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

SnukValue snuk_interpreter_get_env(SnukInterpreter *intpret, SnukStringView name);

bool snuk_interpreter_set_env(SnukInterpreter *intpret, SnukStringView name, SnukValue value);

bool snuk_interpreter_create_var(
    SnukInterpreter *intpret, SnukStringView name, SnukType *type, SnukValue value, bool is_const);

bool snuk_interpreter_value_is_of_type(SnukInterpreter *intpret, SnukValue value, SnukType *type);

