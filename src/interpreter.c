#include "interpreter.h"

#include "memory.h"
#include <string.h>
#include "snuk_string.h"
#include "io.h"
#include "logger.h"

#define GET_SCOPE(rc) ((SnukScope *)snuk_ref_counter_get(rc))

/**
 * @brief Allocate a SnukEnv and populate it by evaluating the given expression.
 */
SNUK_INLINE SnukEnv *snuk_create_env(SnukInterpreter *intpret, SnukStringView name, SnukExpr *val) {
    SnukEnv *env = (SnukEnv *)snuk_alloc(sizeof(SnukEnv), alignof(SnukEnv));

    SnukValue value = snuk_interpreter_eval_expr(intpret, val);

    *env = (SnukEnv){
        .name = name,
        .value = snuk_interpreter_copy_value(value),
    };

    snuk_interpreter_free_value(value);

    return env;
}

SNUK_INLINE void snuk_free_env(SnukEnv *env) {
    if (!env) return;

    snuk_interpreter_free_value(env->value);

    snuk_free(env);
}

SNUK_INLINE void loop_and_destroy_envs(SnukScope *scope) {
    uint64_t count = snuk_darray_get_length(scope->vars);
    for (uint64_t i = 0; i < count; ++i) {
        SnukEnv *env;
        snuk_darray_pop(&scope->vars, &env);
        snuk_free_env(env);
    }
}

/**
 * @brief Refcount finalizer for a SnukScope.
 *
 * Destroys the bindings darray, releases the parent reference, and frees the
 * scope allocation. Used as the destructor callback when wrapping a scope
 * with snuk_ref_counter_create.
 *
 * @param data Unused user data pointer required by the refcount finalizer
 * signature.
 * @param ptr Pointer to the SnukScope being released.
 */
SNUK_INLINE void snuk_free_scope(void *data, void *ptr) {
    SNUK_UNUSED(data);
    SnukScope *scope = (SnukScope *)ptr;

    loop_and_destroy_envs(scope);

    snuk_darray_destroy(scope->vars);

    if (scope->parent) snuk_ref_counter_release(scope->parent);

    snuk_free(scope);
}

/**
 * @brief Allocate a new scope and wrap it in a refcounter.
 *
 * @param parent Parent scope reference, consumed by this call. Pass NULL to
 * create a root scope.
 *
 * @return Refcounted handle to the new scope, with snuk_scope_free as the
 * finalizer.
 */
SNUK_INLINE SnukRefCounter *snuk_create_scope(SnukRefCounter *parent) {
    SnukScope *scope = (SnukScope *)snuk_alloc(sizeof(SnukScope), alignof(SnukScope));
    *scope = (SnukScope){
        .vars = snuk_darray_create(SnukEnv *, NULL),
        .parent = snuk_ref_counter_move(&parent),
    };
    return snuk_ref_counter_create(scope, NULL, snuk_free_scope);
}

/**
 * @brief Push a new child scope and make it the interpreter's current scope.
 */
SNUK_INLINE void snuk_scope_push(SnukInterpreter *intpret) {
    intpret->current = snuk_create_scope(snuk_ref_counter_move(&intpret->current));
}

/**
 * @brief Pop the current scope and restore its parent as the active scope.
 */
SNUK_INLINE void snuk_scope_pop(SnukInterpreter *intpret) {
    if (intpret->global == intpret->current) SNUK_SHOULD_NOT_REACH_HERE;

    SnukScope *scope = GET_SCOPE(intpret->current);
    SnukRefCounter *parent = snuk_ref_counter_retain(scope->parent);

    snuk_ref_counter_release(intpret->current);
    intpret->current = snuk_ref_counter_move(&parent);
}

/**
 * @brief Append a binding to a scope's variable list.
 */
SNUK_INLINE SnukEnv *snuk_scope_add_env(SnukScope *scope, SnukEnv *env) {
    // TODO: multiple declaration errors

    snuk_darray_push(&scope->vars, env);

    return env;
}

/**
 * @brief Find a binding by name within a single scope, without walking parents.
 */
SNUK_INLINE SnukEnv *snuk_scope_lookup(SnukScope *scope, SnukStringView name) {
    uint64_t count = snuk_darray_get_length(scope->vars);
    for (uint64_t j = 0; j < count; ++j) {
        if (name.len != scope->vars[j]->name.len) continue;
        if (snuk_string_n_equal(name.str, scope->vars[j]->name.str, name.len))
            return scope->vars[j];
    }
    return NULL;
}

/**
 * @brief Walk the scope chain from current to global to resolve a name.
 */
SNUK_INLINE SnukEnv *snuk_env_lookup(SnukInterpreter *intpret, SnukStringView name) {
    SnukRefCounter *rc = intpret->current;
    SnukEnv *env;
    while (rc) {
        SnukScope *scope = GET_SCOPE(rc);
        if ((env = snuk_scope_lookup(scope, name))) return env;
        rc = scope->parent;
    }
    return NULL;
}

/**
 * @brief Coerce a runtime value to a boolean for conditions and loops.
 */
SNUK_INLINE bool is_true_value(SnukValue value) {
    switch (value.type) {
        case SNUK_VALUE_UNKOWN:
        case SNUK_VALUE_NULL:
            return false;
        case SNUK_VALUE_BOOL:
            return value.bool_value;
        case SNUK_VALUE_INT:
            return value.int_value != 0;
        case SNUK_VALUE_FLOAT:
            return value.float_value != 0;
        case SNUK_VALUE_STRING:
            return value.string_value.len != 0;

        case SNUK_VALUE_FN:
            return true;

        // TODO:
        case SNUK_VALUE_TYPE:
        default:
            return false;
    }
}

static SnukValue get_unary_value(SnukInterpreter *intpret, SnukExpr *expr);
static SnukValue get_binary_value(SnukInterpreter *intpret, SnukExpr *expr);
static SnukValue perform_binary_op(SnukValue left, SnukValue right, SnukTokenType op);

static void print_exprs(SnukInterpreter *intpret, SnukExpr **exprs);

static SnukValue execute_block_expr(SnukInterpreter *intpret, SnukExpr *block, int capture_signals, int propogate_signals);
static SnukValue execute_if_expr(SnukInterpreter *intpret, SnukExpr *expr);
static SnukValue execute_while_expr(SnukInterpreter *intpret, SnukExpr *loop);
static SnukValue execute_for_expr(SnukInterpreter *intpret, SnukExpr *loop);
static SnukValue execute_fn_expr(SnukInterpreter *intpret, SnukExpr *expr);

void snuk_interpreter_init(SnukInterpreter *intpret) {
    *intpret = (SnukInterpreter){
        .global = snuk_create_scope(NULL),
        .signal = SNUK_SIGNAL_NONE,
    };
    intpret->current = snuk_ref_counter_retain(intpret->global);
}

void snuk_interpreter_deinit(SnukInterpreter *intpret) {
    if (!intpret) return;

    SnukRefCounter *rc = intpret->current;
    while (rc) {
        SnukScope *scope = GET_SCOPE(rc);
        loop_and_destroy_envs(scope);
        rc = scope->parent;
    }

    snuk_ref_counter_release(intpret->current);
    snuk_ref_counter_release(intpret->global);

    *intpret = (SnukInterpreter){0};
}

SnukValue snuk_interpreter_copy_value(SnukValue value) {
    switch (value.type) {
        case SNUK_VALUE_FN:
            value.fn_value.closure = snuk_ref_counter_retain(value.fn_value.closure);
            break;

        case SNUK_VALUE_TYPE:
        case SNUK_VALUE_UNKOWN:
        case SNUK_VALUE_INT:
        case SNUK_VALUE_FLOAT:
        case SNUK_VALUE_BOOL:
        case SNUK_VALUE_STRING:
        case SNUK_VALUE_NULL:
        case SNUK_VALUE_MAX:
        default:
            break;
    }

    return value;
}

void snuk_interpreter_free_value(SnukValue value) {
    switch (value.type) {
        case SNUK_VALUE_FN:
            snuk_ref_counter_release(value.fn_value.closure);
            break;

        case SNUK_VALUE_TYPE:
        case SNUK_VALUE_UNKOWN:
        case SNUK_VALUE_INT:
        case SNUK_VALUE_FLOAT:
        case SNUK_VALUE_BOOL:
        case SNUK_VALUE_STRING:
        case SNUK_VALUE_NULL:
        case SNUK_VALUE_MAX:
        default:
            break;
    }
}

/**
 * @brief Execute a top-level parsed item.
 */
SnukValue snuk_interpreter_exec_item(SnukInterpreter *intpret, SnukItem *item) {
    switch (item->type) {
        case SNUK_ITEM_EXPR:
            return snuk_interpreter_eval_expr(intpret, item->expr);

        case SNUK_ITEM_VAR_DECL:
        case SNUK_ITEM_CONST_DECL:
            {
                // TODO: const
                SnukEnv *env = snuk_create_env(intpret, item->decl_item.name, item->decl_item.expr);
                SnukValue value = snuk_scope_add_env(GET_SCOPE(intpret->current), env)->value;
                return snuk_interpreter_copy_value(value);
            }

            // TODO: Stroing types
        case SNUK_ITEM_TYPE_DECL:
            break;

        case SNUK_ITEM_PRINT:
            print_exprs(intpret, item->print_exprs);
            // TODO: return something else?
            return (SnukValue){.type = SNUK_VALUE_NULL};
            break;

        // TODO:
        case SNUK_ITEM_RETURN:
        case SNUK_ITEM_BREAK:
            {
                SnukValue value = {.type = SNUK_VALUE_NULL};
                if (item->expr)
                    value = snuk_interpreter_eval_expr(intpret, item->expr);
                intpret->signal = item->type == SNUK_ITEM_RETURN ? SNUK_SIGNAL_RETURN : SNUK_SIGNAL_BREAK;
                return value;
            }
        case SNUK_ITEM_CONTINUE:
            intpret->signal = SNUK_SIGNAL_CONTINUE;
            return (SnukValue){.type = SNUK_VALUE_NULL};

        case SNUK_ITEM_MAX:
        default:
            break;
    }
    return (SnukValue){.type = SNUK_VALUE_UNKOWN};
}

/**
 * @brief Evaluate an expression and return its runtime value.
 */
SnukValue snuk_interpreter_eval_expr(SnukInterpreter *intpret, SnukExpr *expr) {
    switch (expr->type) {
        case SNUK_EXPR_IDENTIFIER:
            {
                SnukEnv *env = snuk_env_lookup(intpret, expr->identifier);
                if (!env) return (SnukValue){.type = SNUK_VALUE_UNKOWN};
                return snuk_interpreter_copy_value(env->value);
            }

        case SNUK_EXPR_INT:
            return (SnukValue){
                .type = SNUK_VALUE_INT,
                .int_value = expr->int_literal,
            };

        case SNUK_EXPR_FLOAT:
            return (SnukValue){
                .type = SNUK_VALUE_FLOAT,
                .float_value = expr->float_literal,
            };

        case SNUK_EXPR_STRING:
            return (SnukValue){
                .type = SNUK_VALUE_STRING, 
                .string_value = expr->string_literal,
            };

        case SNUK_EXPR_BOOL:
            return (SnukValue){
                .type = SNUK_VALUE_BOOL,
                .bool_value = expr->bool_literal,
            };

        case SNUK_EXPR_NULL:
            return (SnukValue){
                .type = SNUK_VALUE_NULL,
            };

        case SNUK_EXPR_UNARY:
            return get_unary_value(intpret, expr);

        case SNUK_EXPR_BINARY:
            return get_binary_value(intpret, expr);

        case SNUK_EXPR_ASSIGN:
            {
                SnukValue value = snuk_interpreter_eval_expr(intpret, expr->assign.value);
                snuk_env_lookup(intpret, expr->assign.identifier->identifier)->value =
                    snuk_interpreter_copy_value(value);
                return value;
            }

        // TODO: Compound assign
        case SNUK_EXPR_COMPOUND_ASSIGN:
            break;

        case SNUK_EXPR_IF:
            return execute_if_expr(intpret, expr);

        // TODO: match
        case SNUK_EXPR_MATCH:
            break;

        case SNUK_EXPR_WHILE:
        case SNUK_EXPR_DO_WHILE:
            return execute_while_expr(intpret, expr);

        case SNUK_EXPR_FOR:
            return execute_for_expr(intpret, expr);

        case SNUK_EXPR_FN:
            {
                SnukValue value = {
                    .type = SNUK_VALUE_FN,
                    .fn_value = {
                        .closure = snuk_ref_counter_retain(intpret->current),
                        .body = expr->fn_expr.body,
                        .params = expr->fn_expr.params,
                        .return_type = expr->fn_expr.return_type,
                    },
                };

                // Syntax sugar
                if (expr->fn_expr.name.len) {
                    SnukStringView name = expr->fn_expr.name;
                    expr->fn_expr.name = (SnukStringView){0};
                    SnukEnv *env = snuk_create_env(intpret, name, expr);
                    snuk_scope_add_env(GET_SCOPE(intpret->current), env);
                }

                return value;
            }

        case SNUK_EXPR_TYPE:
            break;

        case SNUK_EXPR_BLOCK:
            return execute_block_expr(intpret, expr, SNUK_SIGNAL_BREAK, SNUK_SIGNAL_NONE);

        // TODO:
        case SNUK_EXPR_CALL:
            return execute_fn_expr(intpret, expr);

        case SNUK_EXPR_MEMBER:
            break;
        case SNUK_EXPR_INDEX:
            break;

        // ignoring comments for now
        case SNUK_EXPR_LINE_COMMENT:
        case SNUK_EXPR_BLOCK_COMMENT:
        case SNUK_EXPR_MAX:
        default:
            break;
    }

    return (SnukValue){.type = SNUK_VALUE_UNKOWN};
}

/**
 * @brief Evaluate a unary expression's operand and apply the operator.
 */
static SnukValue get_unary_value(SnukInterpreter *intpret, SnukExpr *expr) {
    SnukValue val = snuk_interpreter_eval_expr(intpret, expr->unary.operand);

    switch (expr->unary.op) {
        case SNUK_TOKEN_PLUS:
            return val;

        case SNUK_TOKEN_MINUS:
            switch (val.type) {
                case SNUK_VALUE_INT:
                    val.int_value *= -1;
                    break;
                case SNUK_VALUE_FLOAT:
                    val.float_value *= -1;
                    break;
                default:
                    // TODO: Errors
                    break;
            }
            return val;

        case SNUK_TOKEN_BANG:
        case SNUK_TOKEN_KW_NOT:
            {
                bool bool_value = !is_true_value(val);
                snuk_interpreter_free_value(val);
                val = (SnukValue){
                    .type = SNUK_VALUE_BOOL,
                    .bool_value = bool_value,
                };
                return val;
            }

        case SNUK_TOKEN_TILDE:
            switch (val.type) {
                case SNUK_VALUE_INT:
                    val.int_value = ~val.int_value;
                    return val;
                default:
                    // TODO: Errors
                    break;
            }
            return val;

        default:
            break;
    }

    snuk_interpreter_free_value(val);
    return (SnukValue){.type = SNUK_VALUE_UNKOWN};
}

/**
 * @brief Apply a binary operator to two numeric values, promoting int to float as needed.
 */
static SnukValue perform_binary_op(SnukValue left, SnukValue right, SnukTokenType op) {
    if (left.type != right.type) goto fail;

    SnukValue res = {.type = SNUK_VALUE_UNKOWN};

    if (left.type == SNUK_VALUE_INT || left.type == SNUK_VALUE_FLOAT) {
        res.type = left.type;
        switch (op) {
            case SNUK_TOKEN_PLUS:
                if (left.type == SNUK_VALUE_INT) res.int_value = left.int_value + right.int_value;
                else res.float_value = left.float_value + right.float_value;
                return res;

            case SNUK_TOKEN_MINUS:
                if (res.type == SNUK_VALUE_INT) res.int_value = left.int_value - right.int_value;
                else res.float_value = left.float_value - right.float_value;
                return res;

            case SNUK_TOKEN_STAR:
                if (res.type == SNUK_VALUE_INT) res.int_value = left.int_value * right.int_value;
                else res.float_value = left.float_value * right.float_value;
                return res;

            case SNUK_TOKEN_SLASH:
                if (res.type == SNUK_VALUE_INT) res.int_value = left.int_value / right.int_value;
                else res.float_value = left.float_value / right.float_value;
                return res;

            case SNUK_TOKEN_LESS:
            case SNUK_TOKEN_GREATER_EQUAL:
                res.type = SNUK_VALUE_BOOL;
                if (left.type == SNUK_VALUE_INT) res.bool_value = left.int_value < right.int_value;
                else res.bool_value = left.float_value < right.float_value;
                if (op == SNUK_TOKEN_GREATER_EQUAL) res.bool_value = !res.bool_value;
                return res;

            case SNUK_TOKEN_GREATER:
            case SNUK_TOKEN_LESS_EQUAL:
                res.type = SNUK_VALUE_BOOL;
                if (left.type == SNUK_VALUE_INT) res.bool_value = left.int_value > right.int_value;
                else res.bool_value = left.float_value > right.float_value;
                if (op == SNUK_TOKEN_LESS_EQUAL) res.bool_value = !res.bool_value;
                return res;

            default:
                break;
        }
    }

    if (left.type == SNUK_VALUE_INT) {
        res.type = left.type;
        switch (op) {
            case SNUK_TOKEN_PERCENT:
                res.int_value = left.int_value % right.int_value;
                return res;

            case SNUK_TOKEN_PIPE:
                res.int_value = left.int_value | right.int_value;
                return res;

            case SNUK_TOKEN_CARET:
                res.int_value = left.int_value ^ right.int_value;
                return res;

            case SNUK_TOKEN_AMP:
                res.int_value = left.int_value & right.int_value;
                return res;

            case SNUK_TOKEN_LSHIFT:
                res.int_value = left.int_value << right.int_value;
                return res;

            case SNUK_TOKEN_RSHIFT:
                res.int_value = left.int_value >> right.int_value;
                return res;

            default:
                break;
        }
    }

    switch (op) {
        // TODO: comparsions
        case SNUK_TOKEN_EQUAL:
        case SNUK_TOKEN_BANG_EQUAL:
            res = (SnukValue){
                .type = SNUK_VALUE_BOOL,
            };
            switch (left.type) {
                case SNUK_VALUE_UNKOWN:
                case SNUK_VALUE_NULL:
                    res.bool_value = false;
                    break;
                case SNUK_VALUE_INT:
                    res.bool_value = left.int_value == right.int_value;
                    break;
                case SNUK_VALUE_FLOAT:
                    res.bool_value = left.float_value == right.float_value;
                    break;
                case SNUK_VALUE_BOOL:
                    res.bool_value = left.bool_value == right.bool_value;
                    break;
                case SNUK_VALUE_STRING:
                    res.bool_value =
                        (left.string_value.len == right.string_value.len)
                        && snuk_string_n_equal(left.string_value.str, right.string_value.str, left.string_value.len);
                    break;

                // TODO:
                case SNUK_VALUE_FN:
                    break;
                // TODO:
                case SNUK_VALUE_TYPE:
                    break;

                case SNUK_VALUE_MAX:
                default:
                    SNUK_SHOULD_NOT_REACH_HERE;
                    break;
            }
            if (op == SNUK_TOKEN_BANG_EQUAL) res.bool_value = !res.bool_value;
            return res;

        case SNUK_TOKEN_PLUS:
            if (left.type == SNUK_VALUE_STRING) {
                // Trick to remove quotes
                left.string_value.len--;
                right.string_value.len--;
                right.string_value.str++;
                return (SnukValue){
                    .type = SNUK_VALUE_STRING,
                    .string_value = snuk_string_view_concat(left.string_value, right.string_value),
                };
            }
            break;

        default:
            break;
    }

fail:
    return (SnukValue){.type = SNUK_VALUE_UNKOWN};
}

/**
 * @brief Evaluate both operands of a binary expression and combine them with perform_binary_op.
 */
static SnukValue get_binary_value(SnukInterpreter *intpret, SnukExpr *expr) {
    switch (expr->binary.op) {
        case SNUK_TOKEN_PIPE_PIPE:
        case SNUK_TOKEN_KW_OR:
            {
                SnukValue val = snuk_interpreter_eval_expr(intpret, expr->binary.left);
                bool bool_value = is_true_value(val);
                snuk_interpreter_free_value(val);
                if (!bool_value) {
                    val = snuk_interpreter_eval_expr(intpret, expr->binary.right);
                    bool_value = is_true_value(val);
                    snuk_interpreter_free_value(val);
                }

                return (SnukValue){
                    .type = SNUK_VALUE_BOOL,
                    .bool_value = bool_value,
                };
            }

        case SNUK_TOKEN_AMP_AMP:
        case SNUK_TOKEN_KW_AND:
            {
                SnukValue val = snuk_interpreter_eval_expr(intpret, expr->binary.left);
                bool bool_value = is_true_value(val);
                snuk_interpreter_free_value(val);
                if (bool_value) {
                    val = snuk_interpreter_eval_expr(intpret, expr->binary.right);
                    bool_value = is_true_value(val);
                    snuk_interpreter_free_value(val);
                }

                return (SnukValue){
                    .type = SNUK_VALUE_BOOL,
                    .bool_value = bool_value,
                };
            }

        default:
            break;
    }

    SnukValue left = snuk_interpreter_eval_expr(intpret, expr->binary.left);
    SnukValue right = snuk_interpreter_eval_expr(intpret, expr->binary.right);
    SnukValue res = perform_binary_op(left, right, expr->binary.op);
    snuk_interpreter_free_value(left);
    snuk_interpreter_free_value(right);
    
    // TODO: Errors, type checking
    return res;
}

/**
 * @brief Evaluate each expression in the darray and print its value to stdout.
 */
static void print_exprs(SnukInterpreter *intpret, SnukExpr **exprs) {
    if (!exprs) return;

    uint64_t count = snuk_darray_get_length(exprs);
    uint64_t len;
    for (uint64_t j = 0; j < count; ++j) {
        SnukValue value = snuk_interpreter_eval_expr(intpret, exprs[j]);
        switch (value.type) {
            case SNUK_VALUE_UNKOWN:
                snuk_print("Something went wrong, value was UNKNOWN!");
                break;
            case SNUK_VALUE_INT:
                snuk_print("%ld", value.int_value);
                break;
            case SNUK_VALUE_FLOAT:
                snuk_print("%lf", value.float_value);
                break;
            case SNUK_VALUE_BOOL:
                snuk_print("%s", value.bool_value ? "true" : "false");
                break;
            case SNUK_VALUE_STRING:
                if (value.string_value.len > 2)
                    snuk_print(SNUK_STRING_VIEW_FORMAT, (value.string_value.len - 2), (value.string_value.str + 1));
                break;
            case SNUK_VALUE_NULL:
                snuk_print("null", NULL);
                break;
            case SNUK_VALUE_FN:
                snuk_print("fn:", NULL);
                len = snuk_darray_get_length(value.fn_value.params);
                snuk_print("params: ", NULL);
                for (uint64_t k = 0; k < len; ++k)
                    snuk_print(SNUK_STRING_VIEW_FORMAT", ", SNUK_STRING_VIEW_ARG(value.fn_value.params[k]->name));
                break;
            case SNUK_VALUE_TYPE:
                // TODO:
                snuk_print("type:", NULL);
                break;
            default:
                SNUK_SHOULD_NOT_REACH_HERE;
                break;
        }
        snuk_print(" ", NULL);
        snuk_interpreter_free_value(value);
    }

    snuk_println("", NULL);
}

/**
 * @brief Log a runtime value via the trace logger for debugging.
 */
void snuk_interpreter_log_value(SnukValue value) {
    uint64_t count;
    switch (value.type) {
        case SNUK_VALUE_UNKOWN:
            log_trace("type: %s", SNUK_STRINGIFY(SNUK_VALUE_UNKOWN));
            break;
        case SNUK_VALUE_INT:
            log_trace("type: %s", SNUK_STRINGIFY(SNUK_VALUE_INT));
            log_trace("value: %ld", value.int_value);
            break;
        case SNUK_VALUE_FLOAT:
            log_trace("type: %s", SNUK_STRINGIFY(SNUK_VALUE_FLOAT));
            log_trace("value: %lf", value.float_value);
            break;
        case SNUK_VALUE_BOOL:
            log_trace("type: %s", SNUK_STRINGIFY(SNUK_VALUE_BOOL));
            log_trace("value: %s", value.bool_value ? "true" : "false");
            break;
        case SNUK_VALUE_STRING:
            log_trace("type: %s", SNUK_STRINGIFY(SNUK_VALUE_STRING));
            log_trace("value: "SNUK_STRING_VIEW_FORMAT, SNUK_STRING_VIEW_ARG(value.string_value));
            break;
        case SNUK_VALUE_NULL:
            log_trace("type: %s", SNUK_STRINGIFY(SNUK_VALUE_NULL));
            break;
        case SNUK_VALUE_FN:
            log_trace("type: %s", SNUK_STRINGIFY(SNUK_VALUE_FN));
            count = snuk_darray_get_length(value.fn_value.params);
            log_trace("params: ", NULL);
            for (uint64_t j = 0; j < count; ++j)
                log_trace(SNUK_STRING_VIEW_FORMAT, SNUK_STRING_VIEW_ARG(value.fn_value.params[j]->name));
            break;
        case SNUK_VALUE_TYPE:
            log_trace("type: %s", SNUK_STRINGIFY(SNUK_VALUE_TYPE));
            break;
        default:
            SNUK_SHOULD_NOT_REACH_HERE;
            break;
    }
}

/**
 * @brief Print a runtime value to standard output.
 */
void snuk_interpreter_print_value(SnukValue value) {
    uint64_t count;
    switch (value.type) {
        case SNUK_VALUE_UNKOWN:
            snuk_println("Something went wrong, value was UNKNOWN!");
            break;
        case SNUK_VALUE_INT:
            snuk_println("%ld", value.int_value);
            break;
        case SNUK_VALUE_FLOAT:
            snuk_println("%lf", value.float_value);
            break;
        case SNUK_VALUE_BOOL:
            snuk_println("%s", value.bool_value ? "true" : "false");
            break;
        case SNUK_VALUE_STRING:
            snuk_println(SNUK_STRING_VIEW_FORMAT, SNUK_STRING_VIEW_ARG(value.string_value));
            break;
        case SNUK_VALUE_NULL:
            snuk_println("null", NULL);
            break;
        case SNUK_VALUE_FN:
            snuk_print("fn:", NULL);
            count = snuk_darray_get_length(value.fn_value.params);
            snuk_print("params: ", NULL);
            for (uint64_t j = 0; j < count; ++j)
                snuk_print(SNUK_STRING_VIEW_FORMAT", ", SNUK_STRING_VIEW_ARG(value.fn_value.params[j]->name));
            break;
        case SNUK_VALUE_TYPE:
            // TODO:
            snuk_println("type:", NULL);
            break;
        default:
            SNUK_SHOULD_NOT_REACH_HERE;
            break;
    }
}

/**
 * @brief Execute a block in a fresh scope, capturing signals in the capture mask and propagating those in the propagate mask.
 */
static SnukValue execute_block_expr(SnukInterpreter *intpret, SnukExpr *block, int capture_signals, int propogate_signals) {
    snuk_scope_push(intpret);

    uint64_t count = snuk_darray_get_length(block->block_items);
    SnukValue value = {.type = SNUK_VALUE_NULL};

    for (uint64_t j = 0; j < count; ++j) {
        snuk_interpreter_free_value(value);
        value = snuk_interpreter_exec_item(intpret, block->block_items[j]);

        if (intpret->signal == SNUK_SIGNAL_NONE) continue;

        if (intpret->signal & capture_signals) {
            intpret->signal = SNUK_SIGNAL_NONE;
            break;
        } else if (intpret->signal & propogate_signals) {
            break;
        } else {
            SNUK_SHOULD_NOT_REACH_HERE;
        }
    }

    snuk_scope_pop(intpret);
    return value;
}

/**
 * @brief Evaluate an if/else expression by selecting the matching branch block.
 */
static SnukValue execute_if_expr(SnukInterpreter *intpret, SnukExpr *expr) {
    SnukValue cond = snuk_interpreter_eval_expr(intpret, expr->if_else.condition);
    SnukValue res = {.type = SNUK_VALUE_NULL};

    if (is_true_value(cond)) res = execute_block_expr(intpret, expr->if_else.then_block, SNUK_SIGNAL_NONE, SNUK_SIGNAL_ALL);
    else if (expr->if_else.else_block) res = execute_block_expr(intpret, expr->if_else.else_block, SNUK_SIGNAL_NONE, SNUK_SIGNAL_ALL);

    snuk_interpreter_free_value(cond);
    return res;
}

/**
 * @brief Execute a while or do-while loop, honoring break, continue, and return signals.
 */
static SnukValue execute_while_expr(SnukInterpreter *intpret, SnukExpr *loop) {
    SnukValue res = {.type = SNUK_VALUE_NULL};
    SnukValue cond = {.type = SNUK_VALUE_NULL};

loop_start:
    if (loop->type == SNUK_EXPR_WHILE) {
        snuk_interpreter_free_value(cond);
        cond = snuk_interpreter_eval_expr(intpret, loop->while_loop.condition);
        if (!is_true_value(cond)) goto end;
    }

    snuk_interpreter_free_value(res);
    res = execute_block_expr(intpret, loop->while_loop.body, SNUK_SIGNAL_NONE, SNUK_SIGNAL_ALL);

    switch (intpret->signal) {
        case SNUK_SIGNAL_RETURN:
            // propogate
            goto end;

        case SNUK_SIGNAL_BREAK:
            intpret->signal = SNUK_SIGNAL_NONE;
            goto end;

        case SNUK_SIGNAL_CONTINUE:
            // Nothing to do, iteration continues
            intpret->signal = SNUK_SIGNAL_NONE;
            break;

        case SNUK_SIGNAL_NONE:
        default:
            break;
    }

    if (loop->type == SNUK_EXPR_DO_WHILE) {
        snuk_interpreter_free_value(cond);
        cond = snuk_interpreter_eval_expr(intpret, loop->while_loop.condition);
        if (!is_true_value(cond)) goto end;
    }

    goto loop_start;

end:
    snuk_interpreter_free_value(cond);
    return res;
}

/**
 * @brief Execute a for loop within its own scope, running the init, condition, body, and update steps.
 */
static SnukValue execute_for_expr(SnukInterpreter *intpret, SnukExpr *loop) {
    snuk_scope_push(intpret);
    SnukValue res = {.type = SNUK_VALUE_NULL};
    SnukValue cond = {.type = SNUK_VALUE_NULL};

    if (loop->for_loop.init) {
        SnukValue val = snuk_interpreter_exec_item(intpret, loop->for_loop.init);
        snuk_interpreter_free_value(val);
    }

loop_start:
    if (loop->for_loop.condition) {
        snuk_interpreter_free_value(cond);
        cond = snuk_interpreter_eval_expr(intpret, loop->for_loop.condition);
        if (!is_true_value(cond)) goto end;
    }

    snuk_interpreter_free_value(res);
    res = execute_block_expr(intpret, loop->for_loop.body, SNUK_SIGNAL_NONE, SNUK_SIGNAL_ALL);

    switch (intpret->signal) {
        case SNUK_SIGNAL_RETURN:
            // propogate
            goto end;

        case SNUK_SIGNAL_BREAK:
            intpret->signal = SNUK_SIGNAL_NONE;
            goto end;

        case SNUK_SIGNAL_CONTINUE:
            // Nothing to do, iteration continues
            intpret->signal = SNUK_SIGNAL_NONE;
            break;

        case SNUK_SIGNAL_NONE:
        default:
            break;
    }

    if (loop->for_loop.update) {
        SnukValue val = snuk_interpreter_eval_expr(intpret, loop->for_loop.update);
        snuk_interpreter_free_value(val);
    }

    goto loop_start;

end:
    snuk_interpreter_free_value(cond);
    snuk_scope_pop(intpret);
    return res;
}

/**
 * @brief Test whether a parameter name appears in the function's parameter list.
 */
SNUK_INLINE bool name_exists_in_param_list(SnukParam **params, uint64_t count, SnukStringView name) {
    for (uint64_t i = 0; i < count; ++i) {
        if (params[i]->name.len != name.len) continue;

        if (snuk_string_n_equal(params[i]->name.str, name.str, name.len))
            return true;
    }
    return false;
}

/**
 * @brief Bind call arguments to a function's parameters in a new scope and execute its body.
 */
static SnukValue execute_fn_expr(SnukInterpreter *intpret, SnukExpr *expr) {
    SnukValue fn = snuk_interpreter_eval_expr(intpret, expr->call.fn);
    SNUK_ASSERT(fn.type == SNUK_VALUE_FN, "call expression on non function");

    SnukRefCounter *new_scope = snuk_create_scope(snuk_ref_counter_retain(fn.fn_value.closure));

    uint64_t fn_param_count = snuk_darray_get_length(fn.fn_value.params);
    uint64_t call_param_count = snuk_darray_get_length(expr->call.params);
    bool named_params = false;
    for (uint64_t j = 0; j < call_param_count; ++j) {
        SnukExpr *call_param = expr->call.params[j];
        SnukParam *fn_param = fn.fn_value.params[j];

        SnukStringView name;
        SnukExpr *value;
        // TODO:type
        if (call_param->type == SNUK_EXPR_ASSIGN) {
            named_params = true;
            name = call_param->assign.identifier->identifier;
            // TODO: parameter doesn't exists
            SNUK_ASSERT(name_exists_in_param_list(fn.fn_value.params, fn_param_count, name),
                    "parameter doesn't exists");
            value = call_param->assign.value;
        } else if (!named_params && call_param->type != SNUK_EXPR_COMPOUND_ASSIGN) {
            name = fn_param->name;
            value = call_param;
        } else {
            // TODO: once named parameters are given, should not switch back to positional
            SNUK_SHOULD_NOT_REACH_HERE;
        }
        SnukEnv *env = snuk_create_env(intpret, name, value);
        snuk_scope_add_env(GET_SCOPE(new_scope), env);
    }

    // check all parameters are filled, and if not fill with default value or throw error
    for (uint64_t j = 0; j < fn_param_count; ++j) {
        SnukParam *param = fn.fn_value.params[j];
        if (!snuk_scope_lookup(GET_SCOPE(new_scope), param->name)) {
            SNUK_ASSERT(param->default_value, "value is not given for parameter");
            SnukEnv *env = snuk_create_env(intpret, param->name, param->default_value);
            snuk_scope_add_env(GET_SCOPE(new_scope), env);
        }
    }

    SnukRefCounter *temp = snuk_ref_counter_move(&intpret->current);
    intpret->current = snuk_ref_counter_move(&new_scope);

    SnukValue ret = execute_block_expr(intpret, fn.fn_value.body, SNUK_SIGNAL_RETURN, SNUK_SIGNAL_NONE);

    new_scope = snuk_ref_counter_move(&intpret->current);
    intpret->current = snuk_ref_counter_move(&temp);
    snuk_ref_counter_release(new_scope);

    snuk_interpreter_free_value(fn);
    return ret;
}

