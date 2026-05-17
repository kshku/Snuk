#include "interpreter.h"

#include "io.h"
#include "snuk_scope.h"

/**
 * @brief Push a new child scope and make it the interpreter's current scope.
 */
SNUK_INLINE void snuk_scope_push(SnukInterpreter *intpret) {
    intpret->current =
        snuk_create_scope(snuk_ref_counter_move(&intpret->current));
}

/**
 * @brief Pop the current scope and restore its parent as the active scope.
 */
SNUK_INLINE void snuk_scope_pop(SnukInterpreter *intpret) {
    if (intpret->global == intpret->current) SNUK_SHOULD_NOT_REACH_HERE;

    SnukScope *scope = GET_SCOPE(intpret->current);
    SnukRefCounter *parent = snuk_ref_counter_retain(scope->parent);

    snuk_ref_counter_release(&intpret->current);
    intpret->current = snuk_ref_counter_move(&parent);
}

static SnukValue execute_unary_op(SnukInterpreter *intpret, SnukExpr *expr);
static SnukValue execute_binary_op(SnukInterpreter *intpret, SnukExpr *expr);
static SnukValue perform_binary_op(
    SnukValue left, SnukValue right, SnukTokenType op);

static void execute_print_expr(SnukInterpreter *intpret, SnukExpr **exprs);

static SnukValue execute_block_expr(
    SnukInterpreter *intpret, SnukExpr *block, int capture_signals,
    int propogate_signals);
static SnukValue execute_if_expr(SnukInterpreter *intpret, SnukExpr *expr);
static SnukValue execute_while_expr(SnukInterpreter *intpret, SnukExpr *loop);
static SnukValue execute_for_expr(SnukInterpreter *intpret, SnukExpr *loop);
static SnukValue execute_fn_expr(SnukInterpreter *intpret, SnukExpr *expr);
static SnukValue execute_type_declaration(
    SnukInterpreter *intpret, SnukExpr *expr);
static SnukValue execute_inst_creation(
    SnukInterpreter *intpret, SnukExpr *expr);
static SnukValue execute_compound_assign(
    SnukInterpreter *intpret, SnukExpr *expr);

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

    snuk_ref_counter_release(&intpret->current);
    snuk_ref_counter_release(&intpret->global);

    *intpret = (SnukInterpreter){0};
}

SnukEnv *snuk_interpreter_lookup(
    SnukInterpreter *intpret, SnukStringView name) {
    SnukRefCounter *rc = intpret->current;
    SnukEnv *env;
    while (rc) {
        SnukScope *scope = GET_SCOPE(rc);
        if ((env = snuk_scope_lookup(scope, name))) return env;
        rc = scope->parent;
    }
    return NULL;
}

SnukValue snuk_interpreter_exec_item(SnukInterpreter *intpret, SnukItem *item) {
    switch (item->type) {
        case SNUK_ITEM_EXPR:
            return snuk_interpreter_eval_expr(intpret, item->expr);

        case SNUK_ITEM_VAR_DECL:
        case SNUK_ITEM_CONST_DECL: {
            // TODO: const
            SnukValue value =
                snuk_interpreter_eval_expr(intpret, item->decl_item.expr);
            SnukEnv *env = snuk_create_env(item->decl_item.name, value);
            snuk_interpreter_free_value(value);

            SnukEnv *added =
                snuk_scope_add_env(GET_SCOPE(intpret->current), env);

            if (!added) return (SnukValue){.type = SNUK_VALUE_UNKOWN};

            return snuk_interpreter_copy_value(added->value);
        }

        case SNUK_ITEM_PRINT:
            execute_print_expr(intpret, item->print_exprs);
            // TODO: return something else?
            return (SnukValue){.type = SNUK_VALUE_NULL};
            break;

        // TODO:
        case SNUK_ITEM_RETURN:
        case SNUK_ITEM_BREAK: {
            SnukValue value = {.type = SNUK_VALUE_NULL};
            if (item->expr)
                value = snuk_interpreter_eval_expr(intpret, item->expr);
            intpret->signal = item->type == SNUK_ITEM_RETURN
                                ? SNUK_SIGNAL_RETURN
                                : SNUK_SIGNAL_BREAK;
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
        case SNUK_EXPR_IDENTIFIER: {
            SnukEnv *env = snuk_interpreter_lookup(intpret, expr->identifier);
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
            return execute_unary_op(intpret, expr);

        case SNUK_EXPR_BINARY:
            return execute_binary_op(intpret, expr);

        case SNUK_EXPR_ASSIGN: {
            SnukValue value =
                snuk_interpreter_eval_expr(intpret, expr->assign.value);
            snuk_interpreter_lookup(
                intpret, expr->assign.identifier->identifier)
                ->value = snuk_interpreter_copy_value(value);
            return value;
        }

        // TODO: Compound assign
        case SNUK_EXPR_COMPOUND_ASSIGN:
            return execute_compound_assign(intpret, expr);

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

        case SNUK_EXPR_FN: {
            SnukValue value = {
                .type = SNUK_VALUE_FN,
                .fn_value =
                    {
                               .closure = snuk_ref_counter_retain(intpret->current),
                               .body = expr->fn_expr.body,
                               .params = expr->fn_expr.params,
                               .return_type = expr->fn_expr.return_type,
                               },
            };

            // Syntax sugar
            if (expr->fn_expr.name.len) {
                SnukEnv *env = snuk_create_env(expr->fn_expr.name, value);
                snuk_scope_add_env(GET_SCOPE(intpret->current), env);
            }

            return value;
        }

        case SNUK_EXPR_TYPE:
            return execute_type_declaration(intpret, expr);

        case SNUK_EXPR_TYPE_INST:
            return execute_inst_creation(intpret, expr);

        case SNUK_EXPR_BLOCK:
            return execute_block_expr(
                intpret, expr, SNUK_SIGNAL_BREAK, SNUK_SIGNAL_NONE);

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
static SnukValue execute_unary_op(SnukInterpreter *intpret, SnukExpr *expr) {
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
        case SNUK_TOKEN_KW_NOT: {
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
 * @brief Apply a binary operator to two numeric values, promoting int to float
 * as needed.
 */
static SnukValue perform_binary_op(
    SnukValue left, SnukValue right, SnukTokenType op) {
    if (left.type != right.type) goto fail;

    SnukValue res = {.type = SNUK_VALUE_UNKOWN};

    if (left.type == SNUK_VALUE_INT || left.type == SNUK_VALUE_FLOAT) {
        res.type = left.type;
        switch (op) {
            case SNUK_TOKEN_PLUS:
                if (left.type == SNUK_VALUE_INT)
                    res.int_value = left.int_value + right.int_value;
                else res.float_value = left.float_value + right.float_value;
                return res;

            case SNUK_TOKEN_MINUS:
                if (res.type == SNUK_VALUE_INT)
                    res.int_value = left.int_value - right.int_value;
                else res.float_value = left.float_value - right.float_value;
                return res;

            case SNUK_TOKEN_STAR:
                if (res.type == SNUK_VALUE_INT)
                    res.int_value = left.int_value * right.int_value;
                else res.float_value = left.float_value * right.float_value;
                return res;

            case SNUK_TOKEN_SLASH:
                if (res.type == SNUK_VALUE_INT)
                    res.int_value = left.int_value / right.int_value;
                else res.float_value = left.float_value / right.float_value;
                return res;

            case SNUK_TOKEN_LESS:
            case SNUK_TOKEN_GREATER_EQUAL:
                res.type = SNUK_VALUE_BOOL;
                if (left.type == SNUK_VALUE_INT)
                    res.bool_value = left.int_value < right.int_value;
                else res.bool_value = left.float_value < right.float_value;
                if (op == SNUK_TOKEN_GREATER_EQUAL)
                    res.bool_value = !res.bool_value;
                return res;

            case SNUK_TOKEN_GREATER:
            case SNUK_TOKEN_LESS_EQUAL:
                res.type = SNUK_VALUE_BOOL;
                if (left.type == SNUK_VALUE_INT)
                    res.bool_value = left.int_value > right.int_value;
                else res.bool_value = left.float_value > right.float_value;
                if (op == SNUK_TOKEN_LESS_EQUAL)
                    res.bool_value = !res.bool_value;
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
                        && snuk_string_n_equal(
                            left.string_value.str, right.string_value.str,
                            left.string_value.len);
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
                    .string_value = snuk_string_view_concat(
                        left.string_value, right.string_value),
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
 * @brief Evaluate both operands of a binary expression and combine them with
 * perform_binary_op.
 */
static SnukValue execute_binary_op(SnukInterpreter *intpret, SnukExpr *expr) {
    switch (expr->binary.op) {
        case SNUK_TOKEN_PIPE_PIPE:
        case SNUK_TOKEN_KW_OR: {
            SnukValue val =
                snuk_interpreter_eval_expr(intpret, expr->binary.left);
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
        case SNUK_TOKEN_KW_AND: {
            SnukValue val =
                snuk_interpreter_eval_expr(intpret, expr->binary.left);
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
static void execute_print_expr(SnukInterpreter *intpret, SnukExpr **exprs) {
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
                    snuk_print(
                        SNUK_STRING_VIEW_FORMAT, (value.string_value.len - 2),
                        (value.string_value.str + 1));
                break;
            case SNUK_VALUE_NULL:
                snuk_print("null", NULL);
                break;
            case SNUK_VALUE_FN:
                snuk_print("fn:", NULL);
                len = snuk_darray_get_length(value.fn_value.params);
                snuk_print("params: ", NULL);
                for (uint64_t k = 0; k < len; ++k)
                    snuk_print(
                        SNUK_STRING_VIEW_FORMAT ", ",
                        SNUK_STRING_VIEW_ARG(value.fn_value.params[k]->name));
                break;
            case SNUK_VALUE_TYPE:
                snuk_print("type:", NULL);
                break;
            case SNUK_VALUE_TYPE_INST:
                snuk_print("type inst:", NULL);
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
 * @brief Execute a block in a fresh scope, capturing signals in the capture
 * mask and propagating those in the propagate mask.
 */
static SnukValue execute_block_expr(
    SnukInterpreter *intpret, SnukExpr *block, int capture_signals,
    int propogate_signals) {
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
    SnukValue cond =
        snuk_interpreter_eval_expr(intpret, expr->if_else.condition);
    SnukValue res = {.type = SNUK_VALUE_NULL};

    if (is_true_value(cond))
        res = execute_block_expr(
            intpret, expr->if_else.then_block, SNUK_SIGNAL_NONE,
            SNUK_SIGNAL_ALL);
    else if (expr->if_else.else_block)
        res = execute_block_expr(
            intpret, expr->if_else.else_block, SNUK_SIGNAL_NONE,
            SNUK_SIGNAL_ALL);

    snuk_interpreter_free_value(cond);
    return res;
}

/**
 * @brief Execute a while or do-while loop, honoring break, continue, and return
 * signals.
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
    res = execute_block_expr(
        intpret, loop->while_loop.body, SNUK_SIGNAL_NONE, SNUK_SIGNAL_ALL);

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
 * @brief Execute a for loop within its own scope, running the init, condition,
 * body, and update steps.
 */
static SnukValue execute_for_expr(SnukInterpreter *intpret, SnukExpr *loop) {
    snuk_scope_push(intpret);
    SnukValue res = {.type = SNUK_VALUE_NULL};
    SnukValue cond = {.type = SNUK_VALUE_NULL};

    if (loop->for_loop.init) {
        SnukValue val =
            snuk_interpreter_exec_item(intpret, loop->for_loop.init);
        snuk_interpreter_free_value(val);
    }

loop_start:
    if (loop->for_loop.condition) {
        snuk_interpreter_free_value(cond);
        cond = snuk_interpreter_eval_expr(intpret, loop->for_loop.condition);
        if (!is_true_value(cond)) goto end;
    }

    snuk_interpreter_free_value(res);
    res = execute_block_expr(
        intpret, loop->for_loop.body, SNUK_SIGNAL_NONE, SNUK_SIGNAL_ALL);

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
        SnukValue val =
            snuk_interpreter_eval_expr(intpret, loop->for_loop.update);
        snuk_interpreter_free_value(val);
    }

    goto loop_start;

end:
    snuk_interpreter_free_value(cond);
    snuk_scope_pop(intpret);
    return res;
}

/**
 * @brief Test whether a parameter name appears in the function's parameter
 * list.
 */
SNUK_INLINE bool name_exists_in_param_list(
    SnukParam **params, uint64_t count, SnukStringView name) {
    for (uint64_t i = 0; i < count; ++i)
        if (snuk_string_view_equal(params[i]->name, name)) return true;
    return false;
}

/**
 * @brief Bind call arguments to a function's parameters in a new scope and
 * execute its body.
 */
static SnukValue execute_fn_expr(SnukInterpreter *intpret, SnukExpr *expr) {
    SnukValue fn = snuk_interpreter_eval_expr(intpret, expr->call.fn);
    SNUK_ASSERT(fn.type == SNUK_VALUE_FN, "call expression on non function");

    SnukRefCounter *new_scope =
        snuk_create_scope(snuk_ref_counter_retain(fn.fn_value.closure));

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
            SNUK_ASSERT(
                name_exists_in_param_list(
                    fn.fn_value.params, fn_param_count, name),
                "parameter doesn't exists");
            value = call_param->assign.value;
        } else if (
            !named_params && call_param->type != SNUK_EXPR_COMPOUND_ASSIGN) {
            name = fn_param->name;
            value = call_param;
        } else {
            // TODO: once named parameters are given, should not switch back to
            // positional
            SNUK_SHOULD_NOT_REACH_HERE;
        }
        SnukValue v = snuk_interpreter_eval_expr(intpret, value);
        SnukEnv *env = snuk_create_env(name, v);
        snuk_interpreter_free_value(v);
        snuk_scope_add_env(GET_SCOPE(new_scope), env);
    }

    // check all parameters are filled, and if not fill with default value or
    // throw error
    for (uint64_t j = 0; j < fn_param_count; ++j) {
        SnukParam *param = fn.fn_value.params[j];
        if (!snuk_scope_lookup(GET_SCOPE(new_scope), param->name)) {
            SNUK_ASSERT(
                param->default_value, "value is not given for parameter");
            SnukValue v =
                snuk_interpreter_eval_expr(intpret, param->default_value);
            SnukEnv *env = snuk_create_env(param->name, v);
            snuk_interpreter_free_value(v);
            snuk_scope_add_env(GET_SCOPE(new_scope), env);
        }
    }

    SnukRefCounter *temp = snuk_ref_counter_move(&intpret->current);
    intpret->current = snuk_ref_counter_move(&new_scope);

    SnukValue ret = execute_block_expr(
        intpret, fn.fn_value.body, SNUK_SIGNAL_RETURN, SNUK_SIGNAL_NONE);

    new_scope = snuk_ref_counter_move(&intpret->current);
    intpret->current = snuk_ref_counter_move(&temp);
    snuk_ref_counter_release(&new_scope);

    snuk_interpreter_free_value(fn);
    return ret;
}

/**
 * @brief Get name from the item.
 */
SNUK_INLINE SnukStringView get_name_from_item(SnukItem *item) {
    switch (item->type) {
        case SNUK_ITEM_EXPR:
            switch (item->expr->type) {
                case SNUK_EXPR_FN:
                    return item->expr->fn_expr.name;
                case SNUK_EXPR_TYPE:
                    return item->expr->type_expr.name;

                default:
                    SNUK_SHOULD_NOT_REACH_HERE;
                    break;
            }

        case SNUK_ITEM_VAR_DECL:
        case SNUK_ITEM_CONST_DECL:
            return item->decl_item.name;

        default:
            SNUK_SHOULD_NOT_REACH_HERE;
            break;
    }
    return (SnukStringView){0};
}

/**
 * @brief Test whether a member name appear in the type's member list.
 */
SNUK_INLINE bool name_exists_in_member_list(
    SnukItem **members, uint64_t count, SnukStringView name) {
    for (uint64_t i = 0; i < count; ++i) {
        SnukStringView member_name = get_name_from_item(members[i]);
        if (snuk_string_view_equal(member_name, name)) return true;
    }

    return false;
}

static SnukValue execute_type_declaration(
    SnukInterpreter *intpret, SnukExpr *expr) {
    snuk_scope_push(intpret);

    uint64_t count = snuk_darray_get_length(expr->type_expr.members);
    for (uint64_t i = 0; i < count; ++i) {
        SnukValue val =
            snuk_interpreter_exec_item(intpret, expr->type_expr.members[i]);
        snuk_interpreter_free_value(val);
    }

    SnukValue value = {
        .type = SNUK_VALUE_TYPE,
        .closure = snuk_ref_counter_retain(intpret->current),
    };

    snuk_scope_pop(intpret);

    // Syntax sugar
    if (expr->type_expr.name.len) {
        SnukEnv *env = snuk_create_env(expr->type_expr.name, value);
        snuk_scope_add_env(GET_SCOPE(intpret->current), env);
    }

    return value;
}

static SnukValue execute_inst_creation(
    SnukInterpreter *intpret, SnukExpr *expr) {
    SnukValue type =
        snuk_interpreter_eval_expr(intpret, expr->type_inst_expr.type_name);
    SNUK_ASSERT(
        type.type == SNUK_VALUE_TYPE,
        "type instance creation expression on non type");

    SnukRefCounter *new_scope =
        snuk_create_scope(snuk_ref_counter_retain(type.closure));

    uint64_t init_count = snuk_darray_get_length(expr->type_inst_expr.init);

    for (uint64_t i = 0; i < init_count; ++i) {
        SnukExpr *assign = expr->type_inst_expr.init[i];
        SNUK_ASSERT(
            assign->type == SNUK_EXPR_ASSIGN, "Expected assign expressions");
        SnukStringView name = assign->assign.identifier->identifier;
        SNUK_ASSERT(
            snuk_scope_lookup(GET_SCOPE(type.closure), name) != NULL,
            "member doesn't exists");
        SnukValue v = snuk_interpreter_eval_expr(intpret, assign->assign.value);
        SnukEnv *env = snuk_create_env(name, v);
        snuk_interpreter_free_value(v);
        snuk_scope_add_env(GET_SCOPE(new_scope), env);
    }

    SnukValue value = {
        .type = SNUK_VALUE_TYPE_INST,
        .closure = snuk_ref_counter_move(&new_scope),
    };

    snuk_interpreter_free_value(type);
    return value;
}

static SnukValue execute_compound_assign(
    SnukInterpreter *intpret, SnukExpr *expr) {
    SnukTokenType op_type;
    switch (expr->compound_assign.op) {
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

    SnukValue lhs =
        snuk_interpreter_eval_expr(intpret, expr->compound_assign.identifier);

    SnukValue rhs =
        snuk_interpreter_eval_expr(intpret, expr->compound_assign.value);

    SnukValue res = perform_binary_op(lhs, rhs, op_type);

    snuk_interpreter_lookup(
        intpret, expr->compound_assign.identifier->identifier)
        ->value = snuk_interpreter_copy_value(res);

    snuk_interpreter_free_value(lhs);
    snuk_interpreter_free_value(rhs);

    return res;
}

