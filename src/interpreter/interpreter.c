#include "interpreter.h"

#include "io.h"
#include "parser/snuk_var.h"
#include "snuk_scope.h"

#include <stdio.h>

typedef struct PredefinedTypes {
    const char *type;
    SnukValueType val_type;
} PredefinedTypes;

static PredefinedTypes pre_def_types[] = {
    {.type = "int",   .val_type = SNUK_VALUE_INT   },
    {.type = "float", .val_type = SNUK_VALUE_FLOAT },
    {.type = "bool",  .val_type = SNUK_VALUE_BOOL  },
    {.type = "str",   .val_type = SNUK_VALUE_STRING},
};

SNUK_INLINE SnukValueType get_predef_type(SnukStringView name) {
    for (uint64_t i = 0; i < SNUK_ARRAY_LENGTH(pre_def_types); ++i)
        if (snuk_string_view_equal_cstr(name, pre_def_types[i].type))
            return pre_def_types[i].val_type;
    return SNUK_VALUE_UNKOWN;
}

/**
 * @brief Walk the scope chain from current to global to resolve a name.
 */
SNUK_INLINE SnukEnv *interpreter_lookup(SnukInterpreter *intpret, SnukStringView name) {
    return snuk_scope_lookup_recursive(intpret->current, name);
}

/**
 * @brief Push a new child scope and make it the interpreter's current scope.
 */
SNUK_INLINE void interpreter_push_scope(SnukInterpreter *intpret) {
    intpret->current = snuk_scope_create(snuk_ref_counter_move(&intpret->current), false);
}

/**
 * @brief Pop the current scope and restore its parent as the active scope.
 */
SNUK_INLINE void interpreter_pop_scope(SnukInterpreter *intpret) {
    if (intpret->global == intpret->current) SNUK_SHOULD_NOT_REACH_HERE;

    SnukScope *scope = GET_SCOPE(intpret->current);
    SnukRefCounter *parent = snuk_ref_counter_retain(scope->parent);

    snuk_ref_counter_release(&intpret->current);

    intpret->current = snuk_ref_counter_move(&parent);
}

static void execute_print_item(SnukInterpreter *intpret, SnukExpr **exprs, bool weak_ref);
static SnukValue execute_block_expr(
    SnukInterpreter *intpret, SnukExpr *block, int capture_signals, int propogate_signals, bool weak_ref);
static SnukValue execute_if_expr(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref);
static SnukValue execute_while_expr(SnukInterpreter *intpret, SnukExpr *loop, bool weak_ref);
static SnukValue execute_for_expr(SnukInterpreter *intpret, SnukExpr *loop, bool weak_ref);
static SnukValue execute_type_declaration(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref);
static SnukValue execute_inst_creation(SnukInterpreter *intpret, SnukValue type, SnukExpr *expr, bool weak_ref);
static SnukValue execute_fn_expr(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref);
static SnukValue execute_call_expr(SnukInterpreter *intpret, SnukValue fn, SnukExpr **params, bool weak_ref);
static SnukValue execute_member_access(
    SnukInterpreter *intpret, SnukValue type_or_inst, SnukExpr *field, bool weak_ref);
static SnukValue execute_binary_op(
    SnukInterpreter *intpret, SnukValue left, SnukExpr *right_expr, SnukTokenType op, bool weak_ref);
static SnukValue perform_binary_op(SnukValue left, SnukValue right, SnukTokenType op);
static SnukValue get_binary_op_value(
    SnukInterpreter *intpret, SnukValue lhs, SnukExpr *rhs_expr, SnukTokenType op, bool weak_ref);
static SnukValue execute_unary_op(SnukValue val, SnukTokenType op);
static SnukValue interpreter_exec_item(SnukInterpreter *intpret, SnukItem *item, bool weak_ref);
static SnukValue interpreter_eval_expr(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref);

void snuk_interpreter_init(SnukInterpreter *intpret) {
    *intpret = (SnukInterpreter){
        .global = snuk_scope_create(NULL, false),
        .signal = SNUK_SIGNAL_NONE,
    };
    intpret->current = snuk_ref_counter_retain(intpret->global);
}

void snuk_interpreter_deinit(SnukInterpreter *intpret) {
    if (!intpret) return;

    snuk_ref_counter_release(&intpret->current);
    snuk_ref_counter_release(&intpret->global);

    *intpret = (SnukInterpreter){0};
}

bool snuk_interpreter_value_is_of_type(SnukInterpreter *intpret, SnukValue value, SnukType *type) {
    // in case of parameter without default value, value will be unknown
    if (value.type == SNUK_VALUE_UNKOWN) return true;

    if (value.type == SNUK_VALUE_NULL || type->type == TYPE_ANY) return true;

    if (type->type == TYPE_FN && value.type == SNUK_VALUE_FN)
        return snuk_type_equal(value.fn_value.type, type);

    if (type->type == TYPE_TYPE && (value.type == SNUK_VALUE_TYPE || value.type == SNUK_VALUE_TYPE_INST))
        return snuk_type_equal(value.type_value.type, type);

    if (type->type == TYPE_NAMED) {
        if (value.type == get_predef_type(type->name)) return true;

        if (value.type == SNUK_VALUE_TYPE_INST) return snuk_type_equal(value.type_value.type, type);

        if (value.type != SNUK_VALUE_TYPE) return false;

        SnukEnv *env = interpreter_lookup(intpret, type->name);
        if (!env) return false;

        return snuk_type_equal(value.type_value.type, env->type);
    }

    return false;
}

SnukValue snuk_interpreter_get_env(SnukInterpreter *intpret, SnukStringView name) {
    SnukEnv *env = interpreter_lookup(intpret, name);
    if (!env) return (SnukValue){.type = SNUK_VALUE_UNKOWN};
    return snuk_value_copy(env->value);
}

bool snuk_interpreter_set_env(SnukInterpreter *intpret, SnukStringView name, SnukValue value) {
    SnukEnv *env = interpreter_lookup(intpret, name);
    if (!env) return false;
    if (!snuk_interpreter_value_is_of_type(intpret, value, env->type)) return false;
    snuk_env_assign_value(env, value);
    return true;
}

bool snuk_interpreter_create_env(
    SnukInterpreter *intpret, SnukStringView name, SnukType *type, SnukValue value, bool is_const) {
    // TODO: constant
    SNUK_UNUSED(is_const);
    if (!snuk_interpreter_value_is_of_type(intpret, value, type)) return false;
    SnukEnv *env = snuk_env_create(name, type, value);
    if (!snuk_scope_add_env(intpret->current, env)) return false;
    return true;
}

SnukValue snuk_interpreter_exec_item(SnukInterpreter *intpret, SnukItem *item) {
    return interpreter_exec_item(intpret, item, true);
}

SnukValue snuk_interpreter_eval_expr(SnukInterpreter *intpret, SnukExpr *expr) {
    return interpreter_eval_expr(intpret, expr, true);
}

/**
 * @brief Evaluate a unary expression's operand and apply the operator.
 */
static SnukValue execute_unary_op(SnukValue val, SnukTokenType op) {
    switch (op) {
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
        case SNUK_TOKEN_KW_NOT: {
            bool bool_value = !snuk_value_is_true(val);
            snuk_value_free(val);
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
    return (SnukValue){.type = SNUK_VALUE_UNKOWN};
}

static SnukValue perform_binary_op(SnukValue left, SnukValue right, SnukTokenType op) {
    if (left.type == SNUK_VALUE_STRING || right.type == SNUK_VALUE_STRING) {
        if (left.type == right.type) {
            // Trick to remove quotes
            left.string_value.len--;
            right.string_value.len--;
            right.string_value.str++;
            return (SnukValue){
                .type = SNUK_VALUE_STRING,
                .string_value = snuk_string_view_concat(left.string_value, right.string_value),
            };
        } else {
            SnukValue v = left.type == SNUK_VALUE_STRING ? right : left;
            SnukStringView str;
            char buf[25] = {0};
            uint64_t len = 0;
            switch (v.type) {
                case SNUK_VALUE_NULL:
                    str = snuk_string_view_create_with_len("null", 4);
                    break;
                case SNUK_VALUE_INT:
                    buf[len++] = '"';
                    len = snprintf(buf + 1, SNUK_ARRAY_LENGTH(buf) - 2, PRId64, v.int_value);
                    buf[len++] = '"';
                    str = snuk_string_view_create_with_len(buf, len);
                    break;
                case SNUK_VALUE_FLOAT:
                    buf[len++] = '"';
                    len = snprintf(buf + 1, SNUK_ARRAY_LENGTH(buf) - 2, "%lf", v.float_value);
                    buf[len++] = '"';
                    str = snuk_string_view_create_with_len(buf, len);
                    break;
                case SNUK_VALUE_BOOL:
                    str = v.bool_value ? snuk_string_view_create_with_len("true", 4)
                                       : snuk_string_view_create_with_len("false", 5);
                    break;
                case SNUK_VALUE_FN:
                    str = snuk_string_view_create_with_len("fn", 2);
                    break;
                case SNUK_VALUE_TYPE:
                    str = snuk_string_view_create_with_len("type", 4);
                    break;
                case SNUK_VALUE_TYPE_INST:
                    str = snuk_string_view_create_with_len("type instance", 13);
                    break;
                case SNUK_VALUE_UNKOWN:
                default:
                    goto fail;
            }

            if (left.type == SNUK_VALUE_STRING) {
                left.string_value.len--;
                str.len--;
                str.str++;
                return (SnukValue){
                    .type = SNUK_VALUE_STRING,
                    .string_value = snuk_string_view_concat(left.string_value, str),
                };
            }
            str.len--;
            right.string_value.len--;
            right.string_value.str++;
            return (SnukValue){
                .type = SNUK_VALUE_STRING,
                .string_value = snuk_string_view_concat(str, right.string_value),
            };
        }
    }

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
                    res.bool_value = false;
                    break;
                case SNUK_VALUE_NULL:
                    res.bool_value = right.type == SNUK_VALUE_NULL;
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
                    res.bool_value
                        = (left.string_value.len == right.string_value.len)
                          && snuk_string_n_equal(
                              left.string_value.str, right.string_value.str, left.string_value.len);
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

        default:
            break;
    }

fail:
    return (SnukValue){.type = SNUK_VALUE_UNKOWN};
}

static void interpreter_print_type(SnukType *type) {
    uint64_t count;
    switch (type->type) {
        case TYPE_ANY:
            snuk_print("any", NULL);
            break;

        case TYPE_NAMED:
            snuk_print(SNUK_STRING_VIEW_FORMAT, SNUK_STRING_VIEW_ARG(type->name));
            break;

        case TYPE_FN:
            snuk_print("fn(", NULL);
            count = snuk_darray_get_length(type->fn.param_types);
            for (uint64_t i = 0; i < count; ++i) {
                if (i != 0) snuk_print(", ", NULL);
                interpreter_print_type(type->fn.param_types[i]);
            }
            snuk_print(") -> ", NULL);
            interpreter_print_type(type->fn.return_type);
            break;

        case TYPE_TYPE:
            snuk_print("type {", NULL);
            count = snuk_darray_get_length(type->member_types);
            for (uint64_t i = 0; i < count; ++i) {
                if (i != 0) snuk_print(", ", NULL);
                interpreter_print_type(type->member_types[i]);
            }
            snuk_print("}", NULL);
            break;

        default:
            break;
    }
}

static void interpreter_print_value(SnukValue value) {
    uint64_t len;
    SnukScope *scope;
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
                snuk_print(SNUK_STRING_VIEW_FORMAT, (value.string_value.len - 2),
                           (value.string_value.str + 1));
            break;

        case SNUK_VALUE_NULL:
            snuk_print("null", NULL);
            break;

        case SNUK_VALUE_FN:
            snuk_print("fn(", NULL);
            scope = GET_SCOPE(value.fn_value.closure);
            len = snuk_darray_get_length(scope->vars);
            for (uint64_t i = 0; i < len; ++i) {
                if (i != 0) snuk_print(", ", NULL);
                snuk_print(SNUK_STRING_VIEW_FORMAT ": ", SNUK_STRING_VIEW_ARG(scope->vars[i]->name));
                interpreter_print_type(scope->vars[i]->type);
            }
            snuk_print(") -> ");
            interpreter_print_type(value.fn_value.type->fn.return_type);
            break;

        case SNUK_VALUE_TYPE:
            snuk_print("type {", NULL);
            scope = GET_SCOPE(value.type_value.closure);
            len = snuk_darray_get_length(scope->vars);
            for (uint64_t i = 0; i < len; ++i) {
                if (i != 0) snuk_print("; ", NULL);
                SnukEnv *env = scope->vars[i];
                snuk_print(SNUK_STRING_VIEW_FORMAT ": ", SNUK_STRING_VIEW_ARG(env->name));
                interpreter_print_type(env->type);
            }
            snuk_print("}", NULL);
            break;

        case SNUK_VALUE_TYPE_INST:
            snuk_print("type ", NULL);
            interpreter_print_type(value.type_value.type);
            snuk_print(" {", NULL);
            scope = GET_SCOPE(value.type_value.closure);
            len = snuk_darray_get_length(scope->vars);
            for (uint64_t i = 0; i < len; ++i) {
                if (i != 0) snuk_print(" ", NULL);
                SnukEnv *env = scope->vars[i];
                snuk_print(SNUK_STRING_VIEW_FORMAT ": ", SNUK_STRING_VIEW_ARG(env->name));
                interpreter_print_type(env->type);
                snuk_print(" = ", NULL);
                interpreter_print_value(env->value);
                snuk_print(";", NULL);
            }
            snuk_print("}");
            break;

        default:
            SNUK_SHOULD_NOT_REACH_HERE;
            break;
    }
}

/**
 * @brief Evaluate each expression in the darray and print its value to stdout.
 */
static void execute_print_item(SnukInterpreter *intpret, SnukExpr **exprs, bool weak_ref) {
    if (!exprs) return;

    uint64_t count = snuk_darray_get_length(exprs);
    for (uint64_t i = 0; i < count; ++i) {
        SnukValue value = interpreter_eval_expr(intpret, exprs[i], weak_ref);
        interpreter_print_value(value);
        snuk_print(" ", NULL);
        snuk_value_free(value);
    }

    snuk_println("", NULL);
}

/**
 * @brief Execute a block in a fresh scope, capturing signals in the capture
 * mask and propagating those in the propagate mask.
 */
static SnukValue execute_block_expr(
    SnukInterpreter *intpret, SnukExpr *block, int capture_signals, int propogate_signals, bool weak_ref) {
    interpreter_push_scope(intpret);

    uint64_t count = snuk_darray_get_length(block->block_items);
    SnukValue value = {.type = SNUK_VALUE_NULL};

    for (uint64_t j = 0; j < count; ++j) {
        snuk_value_free(value);
        value = interpreter_exec_item(intpret, block->block_items[j], false);

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

    SnukRefCounter *new_scope = snuk_ref_counter_retain(intpret->current);
    interpreter_pop_scope(intpret);

    if (weak_ref) snuk_scope_downgrade_parent(new_scope);

    snuk_ref_counter_release(&new_scope);

    return value;
}

/**
 * @brief Evaluate an if/else expression by selecting the matching branch block.
 */
static SnukValue execute_if_expr(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref) {
    SnukValue cond = interpreter_eval_expr(intpret, expr->if_else.condition, weak_ref);
    SnukValue res = {.type = SNUK_VALUE_NULL};

    if (snuk_value_is_true(cond)) {
        res = execute_block_expr(intpret, expr->if_else.then_block, SNUK_SIGNAL_NONE, SNUK_SIGNAL_ALL, weak_ref);
    } else if (expr->if_else.else_block) {
        if (expr->if_else.else_block->type == SNUK_EXPR_IF)
            res = execute_if_expr(intpret, expr->if_else.else_block, weak_ref);
        else
            res = execute_block_expr(intpret, expr->if_else.else_block, SNUK_SIGNAL_NONE, SNUK_SIGNAL_ALL, weak_ref);
    }

    snuk_value_free(cond);
    return res;
}

/**
 * @brief Execute a while or do-while loop, honoring break, continue, and return
 * signals.
 */
static SnukValue execute_while_expr(SnukInterpreter *intpret, SnukExpr *loop, bool weak_ref) {
    SnukValue res = {.type = SNUK_VALUE_NULL};
    SnukValue cond = {.type = SNUK_VALUE_NULL};

loop_start:
    if (loop->type == SNUK_EXPR_WHILE) {
        snuk_value_free(cond);
        cond = interpreter_eval_expr(intpret, loop->while_loop.condition, weak_ref);
        if (!snuk_value_is_true(cond)) goto end;
    }

    snuk_value_free(res);
    res = execute_block_expr(intpret, loop->while_loop.body, SNUK_SIGNAL_NONE, SNUK_SIGNAL_ALL, weak_ref);

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
        snuk_value_free(cond);
        cond = interpreter_eval_expr(intpret, loop->while_loop.condition, weak_ref);
        if (!snuk_value_is_true(cond)) goto end;
    }

    goto loop_start;

end:
    snuk_value_free(cond);
    return res;
}

/**
 * @brief Execute a for loop within its own scope, running the init, condition,
 * body, and update steps.
 */
static SnukValue execute_for_expr(SnukInterpreter *intpret, SnukExpr *loop, bool weak_ref) {
    interpreter_push_scope(intpret);
    SnukValue res = {.type = SNUK_VALUE_NULL};
    SnukValue cond = {.type = SNUK_VALUE_NULL};

    if (loop->for_loop.init) {
        SnukValue val = interpreter_exec_item(intpret, loop->for_loop.init, false);
        snuk_value_free(val);
    }

loop_start:
    if (loop->for_loop.condition) {
        snuk_value_free(cond);
        cond = interpreter_eval_expr(intpret, loop->for_loop.condition, false);
        if (!snuk_value_is_true(cond)) goto end;
    }

    snuk_value_free(res);
    res = execute_block_expr(intpret, loop->for_loop.body, SNUK_SIGNAL_NONE, SNUK_SIGNAL_ALL, false);

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
        SnukValue val = interpreter_eval_expr(intpret, loop->for_loop.update, false);
        snuk_value_free(val);
    }

    goto loop_start;

end:
    snuk_value_free(cond);

    SnukRefCounter *new_scope = snuk_ref_counter_retain(intpret->current);
    interpreter_pop_scope(intpret);

    if (weak_ref) snuk_scope_downgrade_parent(new_scope);

    snuk_ref_counter_release(&new_scope);

    return res;
}

static SnukValue execute_type_declaration(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref) {
    interpreter_push_scope(intpret);

    uint64_t count = snuk_darray_get_length(expr->type_expr.members);
    for (uint64_t i = 0; i < count; ++i) {
        SnukValue val = interpreter_exec_item(intpret, expr->type_expr.members[i], true);
        snuk_value_free(val);
    }

    SnukValue value = {
        .type = SNUK_VALUE_TYPE,
        .type_value = {
            .type = expr->type_expr.type,
            .closure = snuk_ref_counter_retain(intpret->current),
            .weak_ref = false,
        },
    };

    interpreter_pop_scope(intpret);

    if (weak_ref) snuk_scope_downgrade_parent(value.type_value.closure);

    // Syntax sugar
    if (expr->type_expr.name.len)
        SNUK_ASSERT(snuk_interpreter_create_env(intpret, expr->type_expr.name, value.type_value.type, value, false),
                    "type name is already used");

    return value;
}

static SnukValue execute_inst_creation(SnukInterpreter *intpret, SnukValue type, SnukExpr *expr, bool weak_ref) {
    SNUK_UNUSED(weak_ref);
    SNUK_ASSERT(type.type == SNUK_VALUE_TYPE, "type instance creation expression on non type");

    interpreter_push_scope(intpret);

    SnukScope *type_scope = GET_SCOPE(type.type_value.closure);
    uint64_t member_count = snuk_darray_get_length(type_scope->vars);
    for (uint64_t i = 0; i < member_count; ++i) {
        SnukEnv *type_env = type_scope->vars[i];
        SnukValue value = snuk_value_copy(type_env->value);
        // When instance is created, things will hold instance as their parent, except the type
        // instance, which still holds it's type as parent
        switch (value.type) {
            case SNUK_VALUE_FN:
                snuk_scope_set_parent(
                    value.fn_value.closure, snuk_ref_counter_retain_weak(intpret->current), true);
                break;

            case SNUK_VALUE_TYPE:
                snuk_scope_set_parent(
                    value.type_value.closure, snuk_ref_counter_retain_weak(intpret->current), true);
                break;

            default:
                break;
        }

        SNUK_ASSERT(snuk_interpreter_create_env(intpret, type_env->name, type_env->type, value, false),
                    "duplicate member value");
        snuk_value_free(value);
    }

    uint64_t init_count = snuk_darray_get_length(expr->type_inst_expr.init);
    for (uint64_t i = 0; i < init_count; ++i) {
        SnukExpr *assign = expr->type_inst_expr.init[i];
        SNUK_ASSERT(assign->type == SNUK_EXPR_ASSIGN, "Expected assign expressions");
        SnukStringView name = assign->assign.identifier->identifier;
        SnukEnv *env = snuk_scope_lookup(intpret->current, name);
        SNUK_ASSERT(env, "member doesn't exists");

        SnukValue value = interpreter_eval_expr(intpret, assign->assign.value, true);
        SNUK_ASSERT(snuk_interpreter_set_env(intpret, name, value), "something went wrong");
        snuk_value_free(value);
    }

    SnukValue value = {
        .type = SNUK_VALUE_TYPE_INST,
        .type_value = {
            .type = expr->type_inst_expr.type,
            .closure = snuk_ref_counter_retain(intpret->current),
            .weak_ref = false,
        },
    };

    interpreter_pop_scope(intpret);

    // Reparent
    snuk_scope_set_parent(value.type_value.closure, snuk_ref_counter_retain(type.type_value.closure), false);

    // Syntax sugar
    if (expr->type_inst_expr.name.len)
        SNUK_ASSERT(snuk_interpreter_create_env(
                        intpret, expr->type_inst_expr.name, value.type_value.type, value, false),
                    "type instance name already exists");

    return value;
}

static SnukValue get_binary_op_value(
    SnukInterpreter *intpret, SnukValue lhs, SnukExpr *rhs_expr, SnukTokenType op, bool weak_ref) {
    SnukValue rhs = interpreter_eval_expr(intpret, rhs_expr, weak_ref);
    SnukTokenType op_type;
    switch (op) {
        case SNUK_TOKEN_PLUS_ASSIGN:
            op_type = SNUK_TOKEN_PLUS;
            break;
        case SNUK_TOKEN_MINUS_ASSIGN:
            op_type = SNUK_TOKEN_MINUS;
            break;
        case SNUK_TOKEN_STAR_ASSIGN:
            op_type = SNUK_TOKEN_STAR;
            break;
        case SNUK_TOKEN_SLASH_ASSIGN:
            op_type = SNUK_TOKEN_SLASH;
            break;
        case SNUK_TOKEN_PERCENT_ASSIGN:
            op_type = SNUK_TOKEN_PERCENT;
            break;
        case SNUK_TOKEN_AMP_ASSIGN:
            op_type = SNUK_TOKEN_AMP;
            break;
        case SNUK_TOKEN_PIPE_ASSIGN:
            op_type = SNUK_TOKEN_PIPE;
            break;
        case SNUK_TOKEN_CARET_ASSIGN:
            op_type = SNUK_TOKEN_CARET;
            break;
        case SNUK_TOKEN_LSHIFT_ASSIGN:
            op_type = SNUK_TOKEN_LSHIFT;
            break;
        case SNUK_TOKEN_RSHIFT_ASSIGN:
            op_type = SNUK_TOKEN_RSHIFT;
            break;
        default:
            break;
    }

    SnukValue res = perform_binary_op(lhs, rhs, op_type);

    snuk_value_free(rhs);
    return res;
}

static SnukValue execute_member_access(
    SnukInterpreter *intpret, SnukValue type_or_inst, SnukExpr *field, bool weak_ref) {
    SNUK_ASSERT(type_or_inst.type == SNUK_VALUE_TYPE || type_or_inst.type == SNUK_VALUE_TYPE_INST, "something is wrong");
    SnukValue res = (SnukValue){.type = SNUK_VALUE_UNKOWN};
    SnukStringView field_name = {0};
    SnukEnv *env;

    switch (field->type) {
        case SNUK_EXPR_IDENTIFIER: {
            field_name = field->identifier;
            env = snuk_scope_lookup(type_or_inst.type_value.closure, field_name);
            SNUK_ASSERT(env, "field doesn't exists");
            res = snuk_value_copy(env->value);
            break;
        }

        case SNUK_EXPR_CALL: {
            SnukStringView self = snuk_string_view_create_with_len("self", 4);
            SNUK_ASSERT(field->call.fn->type == SNUK_EXPR_IDENTIFIER, "something went wrong");
            field_name = field->call.fn->identifier;
            env = snuk_scope_lookup(type_or_inst.type_value.closure, field_name);
            SNUK_ASSERT(env, "field doesn't exists");
            if (type_or_inst.type == SNUK_VALUE_TYPE_INST) {
                // Add self
                SnukEnv *self_env = snuk_env_create(self, type_or_inst.type_value.type, type_or_inst);
                SNUK_ASSERT(snuk_scope_add_env(env->value.fn_value.closure, self_env),
                            "self was "
                            "already "
                            "declared!");
            }

            res = execute_call_expr(intpret, env->value, field->call.params, weak_ref);
            if (type_or_inst.type == SNUK_VALUE_TYPE_INST) {
                // Remove self
                snuk_scope_remove_env(env->value.fn_value.closure, self);
            }

            break;
        }

        case SNUK_EXPR_MEMBER: {
            SNUK_ASSERT(field->member_access.type->type == SNUK_EXPR_IDENTIFIER,
                        "something went "
                        "wrong");
            field_name = field->member_access.type->identifier;
            env = snuk_scope_lookup(type_or_inst.type_value.closure, field_name);
            SNUK_ASSERT(env, "field doesn't exists");
            res = execute_member_access(intpret, env->value, field->member_access.field, weak_ref);
            break;
        }

        case SNUK_EXPR_BINARY:
            SNUK_ASSERT(field->binary.left->type == SNUK_EXPR_IDENTIFIER, "something went wrong");
            field_name = field->binary.left->identifier;
            env = snuk_scope_lookup(type_or_inst.type_value.closure, field_name);
            SNUK_ASSERT(env, "field doesn't exists");
            res = execute_binary_op(intpret, env->value, field->binary.right, field->binary.op, weak_ref);
            break;

        case SNUK_EXPR_UNARY:
            SNUK_ASSERT(field->unary.operand->type == SNUK_EXPR_IDENTIFIER, "something went wrong");
            field_name = field->unary.operand->identifier;
            env = snuk_scope_lookup(type_or_inst.type_value.closure, field_name);
            SNUK_ASSERT(env, "field doesn't exists");
            res = execute_unary_op(env->value, field->unary.op);
            break;

        case SNUK_EXPR_ASSIGN:
            SNUK_ASSERT(field->assign.identifier->type == SNUK_EXPR_IDENTIFIER,
                        "something went "
                        "wrong");
            field_name = field->assign.identifier->identifier;
            env = snuk_scope_lookup(type_or_inst.type_value.closure, field_name);
            SNUK_ASSERT(env, "field doesn't exists");
            res = interpreter_eval_expr(intpret, field->assign.value, weak_ref);
            snuk_env_assign_value(env, res);
            break;

        case SNUK_EXPR_COMPOUND_ASSIGN:
            SNUK_ASSERT(field->compound_assign.identifier->type == SNUK_EXPR_IDENTIFIER,
                        "something"
                        " went "
                        "wrong");
            field_name = field->compound_assign.identifier->identifier;
            env = snuk_scope_lookup(type_or_inst.type_value.closure, field_name);
            SNUK_ASSERT(env, "field doesn't exists");
            res = get_binary_op_value(
                intpret, env->value, field->compound_assign.value, field->compound_assign.op, weak_ref);
            snuk_env_assign_value(env, res);
            break;

        default:
            SNUK_SHOULD_NOT_REACH_HERE;
            break;
    }

    return res;
}

static SnukValue execute_fn_expr(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref) {
    interpreter_push_scope(intpret);

    uint64_t param_count = snuk_darray_get_length(expr->fn_expr.params);
    for (uint64_t i = 0; i < param_count; ++i) {
        SnukVar *param = expr->fn_expr.params[i];
        SnukValue value = (SnukValue){.type = SNUK_VALUE_UNKOWN};
        if (param->value) value = interpreter_eval_expr(intpret, param->value, false);
        SNUK_ASSERT(snuk_interpreter_create_env(intpret, param->name, param->type, value, false),
                    "duplicate parameter names");
        snuk_value_free(value);
    }

    SnukValue value = {
        .type = SNUK_VALUE_FN,
        .fn_value = {
            .closure = snuk_ref_counter_retain(intpret->current),
            .weak_ref = false,
            .body = expr->fn_expr.body,
            .type = expr->fn_expr.type,
        },
    };

    interpreter_pop_scope(intpret);

    if (weak_ref) snuk_scope_downgrade_parent(value.fn_value.closure);

    // Syntax sugar
    if (expr->fn_expr.name.len)
        SNUK_ASSERT(snuk_interpreter_create_env(intpret, expr->fn_expr.name, value.fn_value.type, value, false),
                    "function name is already used");

    return value;
}

/**
 * @brief Bind call arguments to a function's parameters in a new scope and
 * execute its body.
 */
static SnukValue execute_call_expr(SnukInterpreter *intpret, SnukValue fn, SnukExpr **params, bool weak_ref) {
    SNUK_UNUSED(weak_ref);
    SNUK_ASSERT(fn.type == SNUK_VALUE_FN, "call expression on non function");

    interpreter_push_scope(intpret);

    SnukScope *fn_scope = GET_SCOPE(fn.fn_value.closure);
    uint64_t param_count = 0;

    param_count = snuk_darray_get_length(params);
    bool named_params = false;
    for (uint64_t i = 0; i < param_count; ++i) {
        // NOTE: we are storing in darray, so order is maintained
        SnukExpr *param = params[i];
        SnukEnv *fn_env = fn_scope->vars[i];

        SnukStringView name;
        SnukType *type;
        SnukExpr *value;

        if (param->type == SNUK_EXPR_ASSIGN) {
            named_params = true;
            name = param->assign.identifier->identifier;
            fn_env = snuk_scope_lookup(fn.fn_value.closure, name);
            SNUK_ASSERT(fn_env, "parameter doesn't exists");
            type = fn_env->type;
            value = param->assign.value;
        } else if (!named_params && param->type != SNUK_EXPR_COMPOUND_ASSIGN) {
            name = fn_env->name;
            type = fn_env->type;
            value = param;
        } else {
            SNUK_SHOULD_NOT_REACH_HERE;
        }

        SnukValue val = interpreter_eval_expr(intpret, value, false);
        SNUK_ASSERT(snuk_interpreter_create_env(intpret, name, type, val, false),
                    "duplicate "
                    "parameter");
        snuk_value_free(val);
    }

    // check all parameters are filled, and if not fill with default value or
    // throw error
    param_count = snuk_darray_get_length(fn_scope->vars);
    for (uint64_t i = 0; i < param_count; ++i) {
        SnukEnv *fn_env = fn_scope->vars[i];
        SnukEnv *env = snuk_scope_lookup(intpret->current, fn_env->name);
        if (!env) {
            SNUK_ASSERT(fn_env->value.type != SNUK_VALUE_UNKOWN, "parameter was not given");
            SNUK_ASSERT(snuk_interpreter_create_env(intpret, fn_env->name, fn_env->type, fn_env->value, false),
                        "something went wrong");
        }
    }

    // Retain the new scope
    SnukRefCounter *new_scope = snuk_ref_counter_retain(intpret->current);

    interpreter_pop_scope(intpret);

    // Release parent and hold the closure
    snuk_scope_set_parent(new_scope, snuk_ref_counter_retain(fn.fn_value.closure), false);

    SnukRefCounter *temp = snuk_ref_counter_move(&intpret->current);
    intpret->current = snuk_ref_counter_move(&new_scope);

    SnukValue ret = execute_block_expr(intpret, fn.fn_value.body, SNUK_SIGNAL_RETURN, SNUK_SIGNAL_NONE, false);

    new_scope = snuk_ref_counter_move(&intpret->current);
    intpret->current = snuk_ref_counter_move(&temp);

    snuk_ref_counter_release(&new_scope);

    return ret;
}

static SnukValue execute_binary_op(
    SnukInterpreter *intpret, SnukValue left, SnukExpr *right_expr, SnukTokenType op, bool weak_ref) {
    SnukValue right;
    switch (op) {
        case SNUK_TOKEN_PIPE_PIPE:
        case SNUK_TOKEN_KW_OR: {
            bool bool_value = snuk_value_is_true(left);
            if (!bool_value) {
                right = interpreter_eval_expr(intpret, right_expr, weak_ref);
                bool_value = snuk_value_is_true(right);
                snuk_value_free(right);
            }

            return (SnukValue){
                .type = SNUK_VALUE_BOOL,
                .bool_value = bool_value,
            };
        }

        case SNUK_TOKEN_AMP_AMP:
        case SNUK_TOKEN_KW_AND: {
            bool bool_value = snuk_value_is_true(left);
            if (bool_value) {
                right = interpreter_eval_expr(intpret, right_expr, weak_ref);
                bool_value = snuk_value_is_true(right);
                snuk_value_free(right);
            }

            return (SnukValue){
                .type = SNUK_VALUE_BOOL,
                .bool_value = bool_value,
            };
        }

        default:
            break;
    }

    right = interpreter_eval_expr(intpret, right_expr, weak_ref);
    SnukValue res = perform_binary_op(left, right, op);
    snuk_value_free(right);
    return res;
}

static SnukValue interpreter_exec_item(SnukInterpreter *intpret, SnukItem *item, bool weak_ref) {
    switch (item->type) {
        case SNUK_ITEM_EXPR:
            return interpreter_eval_expr(intpret, item->expr, weak_ref);

        case SNUK_ITEM_VAR_DECL:
        case SNUK_ITEM_CONST_DECL: {
            SnukValue value = interpreter_eval_expr(intpret, item->var->value, weak_ref);
            SNUK_ASSERT(snuk_interpreter_create_env(intpret, item->var->name, item->var->type,
                                                    value, item->type == SNUK_ITEM_CONST_DECL),
                        "duplicate variable");
            return value;
        }

        case SNUK_ITEM_PRINT:
            execute_print_item(intpret, item->print_exprs, weak_ref);
            // TODO: return something else?
            return (SnukValue){.type = SNUK_VALUE_NULL};
            break;

        // TODO:
        case SNUK_ITEM_RETURN:
        case SNUK_ITEM_BREAK: {
            SnukValue value = {.type = SNUK_VALUE_NULL};
            if (item->expr) value = interpreter_eval_expr(intpret, item->expr, weak_ref);
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

    SNUK_SHOULD_NOT_REACH_HERE;
    return (SnukValue){.type = SNUK_VALUE_UNKOWN};
}

static SnukValue interpreter_eval_expr(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref) {
    switch (expr->type) {
        case SNUK_EXPR_IDENTIFIER:
            return snuk_interpreter_get_env(intpret, expr->identifier);

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

        case SNUK_EXPR_UNARY: {
            SnukValue val = interpreter_eval_expr(intpret, expr->unary.operand, weak_ref);
            SnukValue ret = execute_unary_op(val, expr->unary.op);
            snuk_value_free(val);
            return ret;
        }

        case SNUK_EXPR_BINARY: {
            SnukValue left = interpreter_eval_expr(intpret, expr->binary.left, weak_ref);
            SnukValue res = execute_binary_op(intpret, left, expr->binary.right, expr->binary.op, weak_ref);
            snuk_value_free(left);

            return res;
        }

        case SNUK_EXPR_ASSIGN: {
            SnukValue value = interpreter_eval_expr(intpret, expr->assign.value, weak_ref);
            SNUK_ASSERT(snuk_interpreter_set_env(intpret, expr->assign.identifier->identifier, value),
                        "failed to set value");
            return value;
        }

        case SNUK_EXPR_COMPOUND_ASSIGN: {
            SnukValue lhs = interpreter_eval_expr(intpret, expr->compound_assign.identifier, weak_ref);
            SnukValue res = get_binary_op_value(
                intpret, lhs, expr->compound_assign.value, expr->compound_assign.op, weak_ref);
            snuk_value_free(lhs);
            SNUK_ASSERT(snuk_interpreter_set_env(intpret, expr->compound_assign.identifier->identifier, res),
                        "failed to set value");
            return res;
        }

        case SNUK_EXPR_IF:
            return execute_if_expr(intpret, expr, weak_ref);

        // TODO: match
        case SNUK_EXPR_MATCH:
            break;

        case SNUK_EXPR_WHILE:
        case SNUK_EXPR_DO_WHILE:
            return execute_while_expr(intpret, expr, weak_ref);

        case SNUK_EXPR_FOR:
            return execute_for_expr(intpret, expr, weak_ref);

        case SNUK_EXPR_FN:
            return execute_fn_expr(intpret, expr, weak_ref);

        case SNUK_EXPR_TYPE:
            return execute_type_declaration(intpret, expr, weak_ref);

        case SNUK_EXPR_TYPE_INST: {
            SnukValue type = snuk_interpreter_get_env(intpret, expr->type_inst_expr.type->name);
            SnukValue ret = execute_inst_creation(intpret, type, expr, weak_ref);
            snuk_value_free(type);
            return ret;
        }

        case SNUK_EXPR_BLOCK:
            return execute_block_expr(intpret, expr, SNUK_SIGNAL_BREAK, SNUK_SIGNAL_NONE, weak_ref);

        case SNUK_EXPR_CALL: {
            SnukValue fn = interpreter_eval_expr(intpret, expr->call.fn, weak_ref);
            SnukValue ret = execute_call_expr(intpret, fn, expr->call.params, weak_ref);
            snuk_value_free(fn);
            return ret;
        }

        case SNUK_EXPR_MEMBER: {
            SnukValue type_or_inst = interpreter_eval_expr(intpret, expr->member_access.type, weak_ref);
            SnukValue ret = execute_member_access(intpret, type_or_inst, expr->member_access.field, weak_ref);
            snuk_value_free(type_or_inst);
            return ret;
        }

        case SNUK_EXPR_SELF: {
            SnukStringView self = snuk_string_view_create_with_len("self", 4);
            SnukValue self_value = snuk_interpreter_get_env(intpret, self);
            SNUK_ASSERT(self_value.type == SNUK_VALUE_TYPE_INST, "self error");
            return self_value;
        }

        case SNUK_EXPR_INDEX:
        case SNUK_EXPR_LIST:
        case SNUK_EXPR_LINE_COMMENT:
        case SNUK_EXPR_BLOCK_COMMENT:
            return (SnukValue){.type = SNUK_VALUE_NULL};

        case SNUK_EXPR_MAX:
        default:
            break;
    }

    SNUK_SHOULD_NOT_REACH_HERE;
    return (SnukValue){.type = SNUK_VALUE_UNKOWN};
}
