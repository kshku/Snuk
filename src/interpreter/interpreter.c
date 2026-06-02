#include "snuk/interpreter/interpreter.h"

#include "snuk/interpreter/builtins/snuk_builtins.h"
#include "snuk/interpreter/interpreter_helper.h"
#include "snuk/interpreter/snuk_scope.h"
#include "snuk/io.h"
#include "snuk/parser/snuk_var.h"

#include <stdio.h>

#define PAGES 10

SNUK_INLINE void *alloc_fn(void *data, uint64_t size, uint64_t align) {
    snLinearAllocator *la = (snLinearAllocator *)data;
    return sn_linear_allocator_allocate(la, size, align);
}

SNUK_INLINE void *realloc_fn(void *data, void *ptr, uint64_t new_size, uint64_t align) {
    snLinearAllocator *la = (snLinearAllocator *)data;
    void *new = sn_linear_allocator_allocate(la, new_size, align);
    memcpy(new, ptr, new_size);
    return new;
}

SNUK_INLINE void free_fn(void *data, void *ptr) {
    SNUK_UNUSED(data);
    SNUK_UNUSED(ptr);
}

SnukStringView self_str = {.str = "self", .len = 4};

SnukStringView value_str = {.str = "value", .len = 5};

static void execute_print_item(SnukInterpreter *intpret, SnukExpr **exprs, bool weak_ref);
static SnukValue execute_if_expr(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref);
static SnukValue execute_while_expr(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref);
static SnukValue execute_for_expr(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref);
static SnukValue execute_type_declaration(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref);
static SnukValue execute_inst_creation(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref);
static SnukValue execute_fn_expr(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref);
static SnukValue execute_call_expr(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref);
static SnukValue execute_binary_op(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref);
static SnukValue perform_binary_op(SnukValue left, SnukValue right, SnukTokenType op);
static SnukValue execute_compound_binary_op(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref);
static SnukValue execute_unary_op(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref);
static SnukValue interpreter_exec_item(SnukInterpreter *intpret, SnukItem *item, bool weak_ref);
static SnukValue interpreter_eval_expr(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref);
static SnukValue execute_assign_expr(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref);
static SnukValue execute_member_get(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref);
static SnukValue execute_extend(SnukInterpreter *intpret, SnukItem *item, bool weak_ref);
static SnukValue execute_interface(SnukInterpreter *intpret, SnukItem *item, bool weak_ref);

void snuk_interpreter_init(SnukInterpreter *intpret) {
    *intpret = (SnukInterpreter){
        .global = snuk_scope_create(NULL, false),
        .signal = SNUK_SIGNAL_NONE,
        .instance = NULL,
        .trash = snuk_darray_create(SnukValue, NULL),
        .mem = snuk_allocate_pages(PAGES),
        .allocator = {
            .data = (void *)&intpret->la,
            .alloc = alloc_fn,
            .realloc = realloc_fn,
            .free = free_fn,
        },
        .panic_mode = false,
    };
    sn_linear_allocator_init(&intpret->la, intpret->mem, PAGES * snuk_page_size());
    intpret->current = snuk_ref_counter_retain(intpret->global);

    // Add builtin types
    snuk_builtins_init(intpret);
    snuk_builtins_create_builtin_types(intpret, true);
}

void snuk_interpreter_deinit(SnukInterpreter *intpret) {
    if (!intpret) return;

    snuk_builtins_deinit(intpret);

    interpreter_clear_trash(intpret);
    snuk_darray_destroy(intpret->trash);

    SNUK_ASSERT(!intpret->instance, "something went wrong");

    snuk_ref_counter_release(&intpret->current);
    snuk_ref_counter_release(&intpret->global);

    sn_linear_allocator_deinit(&intpret->la);
    snuk_free_pages(intpret->mem, PAGES);

    *intpret = (SnukInterpreter){0};
}

bool snuk_interpreter_value_is_of_type(SnukInterpreter *intpret, SnukValue value, SnukType *type) {
    // in case of parameter without default value, value will be unknown
    if (value.type == SNUK_VALUE_UNKOWN || value.type == SNUK_VALUE_NULL) return true;

    if (type->type == TYPE_ANY) return true;

    if (type->type == TYPE_TYPE && value.type == SNUK_VALUE_TYPE)
        return snuk_type_equal(type, value.type_value.type);

    if (type->type == TYPE_NAMED) {
        if (value.type == snuk_builtins_get_value_type(type->name)) return true;
        if (value.type != SNUK_VALUE_TYPE && value.type != SNUK_VALUE_TYPE_INST) return false;

        SnukEnv *env = interpreter_lookup(intpret, type->name);
        if (!env) return false;

        if (env->type->type == TYPE_INTERFACE)
            return snuk_interpreter_value_is_of_type(intpret, value, env->type);

        if (value.type == SNUK_VALUE_TYPE_INST) return snuk_type_equal(type, value.type_value.type);

        if (env->type->type == TYPE_TYPE || env->value.type == SNUK_VALUE_TYPE)
            // Must be having same closure
            return env->value.type_value.closure == value.type_value.closure;

        return false;
    }

    if (type->type == TYPE_FN) {
        if (value.type == SNUK_VALUE_FN) return snuk_type_equal(value.fn_value.type, type);
        if (value.type == SNUK_VALUE_FN_NATIVE) return snuk_type_equal(value.native_fn.type, type);
        return false;
    }

    if (type->type == TYPE_INTERFACE) {
        if (value.type == SNUK_VALUE_INTERFACE) return snuk_type_equal(type, value.interface.type);

        if (value.type == SNUK_VALUE_TYPE || value.type == SNUK_VALUE_TYPE_INST) {
            SnukVar **members = type->members;
            uint64_t count = snuk_darray_get_length(members);
            for (uint64_t i = 0; i < count; ++i) {
                SnukEnv *member = snuk_scope_lookup(value.type_value.closure, members[i]->name);
                if (!member && value.type_value.type_scope)
                    member = snuk_scope_lookup(value.type_value.type_scope, members[i]->name);
                if (!member) return false;
                if (!snuk_interpreter_value_is_of_type(intpret, member->value, members[i]->type)) {
                    return false;
                }
            }
            return true;
        }

        return false;
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
    interpreter_clear_trash(intpret);
    SnukValue res = interpreter_exec_item(intpret, item, true);
    if (intpret->signal != SNUK_SIGNAL_NONE) interpreter_error(intpret, "signal is not none");
    SNUK_INTERPRETER_CHECK(intpret, intpret->signal == SNUK_SIGNAL_NONE, "signal is not none");

    if (intpret->panic_mode) intpret->panic_mode = false;

    return res;
}

SnukValue snuk_interpreter_eval_expr(SnukInterpreter *intpret, SnukExpr *expr) {
    return interpreter_eval_expr(intpret, expr, true);
}

/**
 * @brief Evaluate a unary expression's operand and apply the operator.
 */
static SnukValue execute_unary_op(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref) {
    SnukValue value = interpreter_eval_expr(intpret, expr->unary.operand, weak_ref);
    switch (expr->unary.op) {
        case SNUK_TOKEN_PLUS:
            return value;

        case SNUK_TOKEN_MINUS:
            switch (value.type) {
                case SNUK_VALUE_INT:
                    value.int_value *= -1;
                    break;
                case SNUK_VALUE_FLOAT:
                    value.float_value *= -1;
                    break;
                default:
                    // TODO: Errors
                    break;
            }
            return value;

        case SNUK_TOKEN_BANG:
        case SNUK_TOKEN_KW_NOT: {
            bool bool_value = !snuk_value_is_true(value);
            snuk_value_free(value);
            return (SnukValue){
                .type = SNUK_VALUE_BOOL,
                .bool_value = bool_value,
            };
        }

        case SNUK_TOKEN_TILDE:
            switch (value.type) {
                case SNUK_VALUE_INT:
                    value.int_value = ~value.int_value;
                    break;
                default:
                    // TODO: Errors
                    break;
            }
            return value;

        default:
            break;
    }

    snuk_value_free(value);
    return (SnukValue){.type = SNUK_VALUE_UNKOWN};
}

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

static void interpreter_print_type(SnukType *type) {
    uint64_t count;
    switch (type->type) {
        case TYPE_ANY:
            snuk_print("any", NULL);
            break;
        case TYPE_TYPE:
            snuk_print("type", NULL);
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

        case TYPE_INTERFACE:
            snuk_print("interface", NULL);
            count = snuk_darray_get_length(type->members);
            for (uint64_t i = 0; i < count; ++i) {
                if (i != 0) snuk_print("; ", NULL);
                snuk_print(SNUK_STRING_VIEW_FORMAT ": ", SNUK_STRING_VIEW_ARG(type->members[i]->name));
                interpreter_print_type(type->members[i]->type);
            }
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
                SnukEnv *env = scope->vars[i];
                if (snuk_string_view_equal_cstr(env->name, "self")) continue;
                if (i != 0) snuk_print(" ", NULL);
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
SnukValue execute_block_expr(
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
static SnukValue execute_while_expr(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref) {
    SnukValue res = {.type = SNUK_VALUE_NULL};
    SnukValue cond = {.type = SNUK_VALUE_NULL};

loop_start:
    if (expr->type == SNUK_EXPR_WHILE) {
        snuk_value_free(cond);
        cond = interpreter_eval_expr(intpret, expr->while_loop.condition, weak_ref);
        if (!snuk_value_is_true(cond)) goto end;
    }

    snuk_value_free(res);
    res = execute_block_expr(intpret, expr->while_loop.body, SNUK_SIGNAL_CONTINUE,
                             SNUK_SIGNAL_RETURN | SNUK_SIGNAL_BREAK, weak_ref);

    switch (intpret->signal) {
        case SNUK_SIGNAL_RETURN:
            // propogate
            goto end;

        case SNUK_SIGNAL_BREAK:
            intpret->signal = SNUK_SIGNAL_NONE;
            goto end;

        case SNUK_SIGNAL_CONTINUE:
            SNUK_SHOULD_NOT_REACH_HERE;
            break;

        case SNUK_SIGNAL_NONE:
        default:
            break;
    }

    if (expr->type == SNUK_EXPR_DO_WHILE) {
        snuk_value_free(cond);
        cond = interpreter_eval_expr(intpret, expr->while_loop.condition, weak_ref);
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
static SnukValue execute_for_expr(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref) {
    interpreter_push_scope(intpret);
    SnukValue res = {.type = SNUK_VALUE_NULL};
    SnukValue cond = {.type = SNUK_VALUE_NULL};

    if (expr->for_loop.init) {
        SnukValue val = interpreter_exec_item(intpret, expr->for_loop.init, false);
        SNUK_INTERPRETER_CHECK(intpret, intpret->signal == SNUK_SIGNAL_NONE, "signal is not none");
        snuk_value_free(val);
    }

loop_start:
    if (expr->for_loop.condition) {
        snuk_value_free(cond);
        cond = interpreter_eval_expr(intpret, expr->for_loop.condition, false);
        if (!snuk_value_is_true(cond)) goto end;
    }

    snuk_value_free(res);
    res = execute_block_expr(
        intpret, expr->for_loop.body, SNUK_SIGNAL_CONTINUE, SNUK_SIGNAL_RETURN | SNUK_SIGNAL_BREAK, false);

    switch (intpret->signal) {
        case SNUK_SIGNAL_RETURN:
            // propogate
            goto end;

        case SNUK_SIGNAL_BREAK:
            intpret->signal = SNUK_SIGNAL_NONE;
            goto end;

        case SNUK_SIGNAL_CONTINUE:
            SNUK_SHOULD_NOT_REACH_HERE;
            break;

        case SNUK_SIGNAL_NONE:
        default:
            break;
    }

    if (expr->for_loop.update) {
        SnukValue val = interpreter_eval_expr(intpret, expr->for_loop.update, false);
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
        SNUK_INTERPRETER_CHECK(intpret, intpret->signal == SNUK_SIGNAL_NONE, "signal is not none");
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
        SNUK_INTERPRETER_CHECK(
            intpret,
            snuk_interpreter_create_env(intpret, expr->type_expr.name, value.type_value.type, value, false),
            "type name is already used");

    return value;
}

static SnukValue execute_inst_creation(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref) {
    SnukValue type = snuk_interpreter_get_env(intpret, expr->type_inst_expr.type->name);
    SNUK_INTERPRETER_CHECK(intpret, type.type == SNUK_VALUE_TYPE, "type instance creation expression on non type");

    // We will copy the values from type to instance only when it is assigned.
    // If instance doesn't have a value, but type has:
    // - interpreter_get_member fetches member directly from type.
    // - interpreter_set_member will create a new env in instance scope to put new value.

    interpreter_push_scope(intpret);

    SnukValue value = {
        .type = SNUK_VALUE_TYPE_INST,
        .type_value = {
            .type = expr->type_inst_expr.type,
            .closure = snuk_ref_counter_retain(intpret->current),
            .weak_ref = false,
            .type_scope = snuk_ref_counter_retain(type.type_value.closure),
        },
    };

    uint64_t init_count = snuk_darray_get_length(expr->type_inst_expr.init);
    for (uint64_t i = 0; i < init_count; ++i) {
        SnukExpr *assign = expr->type_inst_expr.init[i];
        SNUK_INTERPRETER_CHECK(intpret, assign->type == SNUK_EXPR_ASSIGN, "Expected assign expressions");
        SnukStringView name = assign->assign.identifier->identifier;
        // The instance itself is the closure, so not adding instance scope
        SnukValue val = interpreter_eval_expr(intpret, assign->assign.value, true);

        // if builtin type, make sure value of value member is right
        SnukValueType val_type = snuk_builtins_get_value_type(value.type_value.type->name);
        if (val_type != SNUK_VALUE_UNKOWN && snuk_string_view_equal(name, value_str))
            SNUK_INTERPRETER_CHECK(intpret, val.type == val_type, "invalid value to the member value");

        SNUK_INTERPRETER_CHECK(intpret, interpreter_set_member(intpret, value, name, val), "failed to initialize member");
        snuk_value_free(val);
    }

    SnukValue self_value = snuk_value_copy(value);
    snuk_ref_counter_downgrade(self_value.type_value.closure);
    self_value.type_value.weak_ref = true;

    SNUK_INTERPRETER_CHECK(
        intpret, snuk_interpreter_create_env(intpret, self_str, self_value.type_value.type, self_value, true),
        "something went wrong while creating self");

    snuk_value_free(self_value);

    interpreter_pop_scope(intpret);

    if (weak_ref) snuk_scope_downgrade_parent(value.type_value.closure);

    // Syntax sugar
    if (expr->type_inst_expr.name.len)
        SNUK_INTERPRETER_CHECK(
            intpret,
            snuk_interpreter_create_env(intpret, expr->type_inst_expr.name, value.type_value.type, value, false),
            "type instance name already exists");

    interpreter_trash(intpret, type);

    return value;
}

static SnukValue execute_compound_binary_op(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref) {
    SnukTokenType op;
    switch (expr->compound_assign.op) {
        case SNUK_TOKEN_PLUS_ASSIGN:
            op = SNUK_TOKEN_PLUS;
            break;
        case SNUK_TOKEN_MINUS_ASSIGN:
            op = SNUK_TOKEN_MINUS;
            break;
        case SNUK_TOKEN_STAR_ASSIGN:
            op = SNUK_TOKEN_STAR;
            break;
        case SNUK_TOKEN_SLASH_ASSIGN:
            op = SNUK_TOKEN_SLASH;
            break;
        case SNUK_TOKEN_PERCENT_ASSIGN:
            op = SNUK_TOKEN_PERCENT;
            break;
        case SNUK_TOKEN_AMP_ASSIGN:
            op = SNUK_TOKEN_AMP;
            break;
        case SNUK_TOKEN_PIPE_ASSIGN:
            op = SNUK_TOKEN_PIPE;
            break;
        case SNUK_TOKEN_CARET_ASSIGN:
            op = SNUK_TOKEN_CARET;
            break;
        case SNUK_TOKEN_LSHIFT_ASSIGN:
            op = SNUK_TOKEN_LSHIFT;
            break;
        case SNUK_TOKEN_RSHIFT_ASSIGN:
            op = SNUK_TOKEN_RSHIFT;
            break;
        default:
            break;
    }

    SnukExpr binary_expr = {
        .type = SNUK_EXPR_BINARY,
        .binary = {
            .op = op,
            .left = expr->compound_assign.identifier,
            .right = expr->compound_assign.value,
        },
    };

    SnukExpr assign_expr = {
        .type = SNUK_EXPR_ASSIGN,
        .assign = {
            .identifier = expr->compound_assign.identifier,
            .value = &binary_expr,
        },
    };

    return interpreter_eval_expr(intpret, &assign_expr, weak_ref);
}

static SnukValue execute_fn_expr(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref) {
    interpreter_push_scope(intpret);

    uint64_t param_count = snuk_darray_get_length(expr->fn_expr.params);
    for (uint64_t i = 0; i < param_count; ++i) {
        SnukVar *param = expr->fn_expr.params[i];
        SnukValue value = (SnukValue){.type = SNUK_VALUE_UNKOWN};
        if (param->value) value = interpreter_eval_expr(intpret, param->value, false);
        SNUK_INTERPRETER_CHECK(
            intpret, snuk_interpreter_create_env(intpret, param->name, param->type, value, false),
            "something went wrong while creating parameter");
        snuk_value_free(value);
    }

    SnukValue value = {
        .type = SNUK_VALUE_FN,
        .fn_value = {
            .closure = snuk_ref_counter_retain(intpret->current),
            .weak_ref = false,
            .instance = NULL,
            .body = expr->fn_expr.body,
            .type = expr->fn_expr.type,
        },
    };

    interpreter_pop_scope(intpret);

    if (weak_ref) snuk_scope_downgrade_parent(value.fn_value.closure);

    // Syntax sugar
    if (expr->fn_expr.name.len)
        SNUK_INTERPRETER_CHECK(
            intpret, snuk_interpreter_create_env(intpret, expr->fn_expr.name, value.fn_value.type, value, false),
            "function name is already used");

    return value;
}

/**
 * @brief Bind call arguments to a function's parameters in a new scope and
 * execute its body.
 */
static SnukValue execute_call_expr(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref) {
    SnukValue fn = interpreter_eval_expr(intpret, expr->call.fn, weak_ref);
    SNUK_INTERPRETER_CHECK(intpret, fn.type == SNUK_VALUE_FN || fn.type == SNUK_VALUE_FN_NATIVE,
                           "call expression on non function");

    SnukRefCounter *fn_scope_rc = NULL;
    if (fn.type == SNUK_VALUE_FN) fn_scope_rc = fn.fn_value.closure;
    else fn_scope_rc = fn.native_fn.closure;

    SnukRefCounter *prev_instance = snuk_ref_counter_move(&intpret->instance);
    if (fn.type == SNUK_VALUE_FN) {
        if (fn.fn_value.instance) intpret->instance = snuk_ref_counter_retain(fn.fn_value.instance);
    } else if (fn.native_fn.instance) {
        intpret->instance = snuk_ref_counter_retain(fn.native_fn.instance);
    }

    interpreter_push_scope(intpret);

    SnukScope *fn_scope = GET_SCOPE(fn_scope_rc);

    uint64_t fn_param_count = snuk_darray_get_length(fn_scope->vars);
    uint64_t param_count = snuk_darray_get_length(expr->call.params);

    SNUK_INTERPRETER_CHECK(intpret, fn_param_count >= param_count, "param count mismatch");

    bool named_params = false;
    for (uint64_t i = 0; i < param_count; ++i) {
        // NOTE: we are storing in darray, so order is maintained
        SnukExpr *param = expr->call.params[i];
        SnukEnv *fn_env = fn_scope->vars[i];

        SnukStringView name;
        SnukType *type;
        SnukExpr *value;

        if (param->type == SNUK_EXPR_ASSIGN) {
            named_params = true;
            name = param->assign.identifier->identifier;
            fn_env = snuk_scope_lookup(fn_scope_rc, name);
            SNUK_INTERPRETER_CHECK(intpret, fn_env, "parameter doesn't exists");
            type = fn_env->type;
            value = param->assign.value;
        } else if (!named_params && param->type != SNUK_EXPR_COMPOUND_ASSIGN) {
            name = fn_env->name;
            type = fn_env->type;
            value = param;
        } else {
            SNUK_INTERPRETER_CHECK(intpret, true, "Parameter error");
        }

        SnukValue val = interpreter_eval_expr(intpret, value, true);
        SNUK_INTERPRETER_CHECK(intpret, snuk_interpreter_create_env(intpret, name, type, val, false),
                               "something went wrong while creating parameter");
        snuk_value_free(val);
    }

    // check all parameters are filled, and if not fill with default value or
    // throw error
    for (uint64_t i = 0; i < fn_param_count; ++i) {
        SnukEnv *fn_env = fn_scope->vars[i];
        SnukEnv *env = snuk_scope_lookup(intpret->current, fn_env->name);
        if (!env) {
            SNUK_INTERPRETER_CHECK(intpret, fn_env->value.type != SNUK_VALUE_UNKOWN, "parameter was not given");
            SNUK_INTERPRETER_CHECK(
                intpret, snuk_interpreter_create_env(intpret, fn_env->name, fn_env->type, fn_env->value, false),
                "something went wrong");
        }
    }

    // Retain the new scope
    SnukRefCounter *new_scope = snuk_ref_counter_retain(intpret->current);

    interpreter_pop_scope(intpret);

    // Release parent and hold the closure
    snuk_scope_set_parent(new_scope, snuk_ref_counter_retain(fn_scope_rc), false);

    SnukRefCounter *temp = snuk_ref_counter_move(&intpret->current);
    intpret->current = snuk_ref_counter_move(&new_scope);

    SnukValue ret;
    if (fn.type == SNUK_VALUE_FN)
        ret = execute_block_expr(intpret, fn.fn_value.body, SNUK_SIGNAL_RETURN, SNUK_SIGNAL_NONE, false);
    else ret = fn.native_fn.fn(intpret);

    new_scope = snuk_ref_counter_move(&intpret->current);
    intpret->current = snuk_ref_counter_move(&temp);

    snuk_ref_counter_release(&new_scope);

    if (intpret->instance) snuk_ref_counter_release(&intpret->instance);
    intpret->instance = snuk_ref_counter_move(&prev_instance);

    interpreter_trash(intpret, fn);

    return ret;
}

static SnukValue execute_binary_op(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref) {
    SnukValue left = interpreter_eval_expr(intpret, expr->binary.left, weak_ref);
    SnukValue right;
    switch (expr->binary.op) {
        case SNUK_TOKEN_PIPE_PIPE:
        case SNUK_TOKEN_KW_OR: {
            bool bool_value = snuk_value_is_true(left);
            snuk_value_free(left);
            if (!bool_value) {
                right = interpreter_eval_expr(intpret, expr->binary.right, weak_ref);
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
            snuk_value_free(left);
            if (bool_value) {
                right = interpreter_eval_expr(intpret, expr->binary.right, weak_ref);
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

    right = interpreter_eval_expr(intpret, expr->binary.right, weak_ref);
    SnukValue res = perform_binary_op(left, right, expr->binary.op);
    snuk_value_free(left);
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
            SNUK_INTERPRETER_CHECK(
                intpret,
                snuk_interpreter_create_env(intpret, item->var->name, item->var->type, value, item->type == SNUK_ITEM_CONST_DECL),
                "something went wrong while creating variable");
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

        case SNUK_ITEM_EXTEND:
            return execute_extend(intpret, item, weak_ref);

        case SNUK_ITEM_INTERFACE:
            return execute_interface(intpret, item, weak_ref);

        case SNUK_ITEM_ERROR:
            log_error("Error: %s", item->error.msg);
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

        case SNUK_EXPR_UNARY:
            return execute_unary_op(intpret, expr, weak_ref);

        case SNUK_EXPR_BINARY:
            return execute_binary_op(intpret, expr, weak_ref);

        case SNUK_EXPR_ASSIGN:
            return execute_assign_expr(intpret, expr, weak_ref);

        case SNUK_EXPR_COMPOUND_ASSIGN:
            return execute_compound_binary_op(intpret, expr, weak_ref);

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

        case SNUK_EXPR_TYPE_INST:
            return execute_inst_creation(intpret, expr, weak_ref);

        case SNUK_EXPR_BLOCK:
            return execute_block_expr(
                intpret, expr, SNUK_SIGNAL_BREAK, SNUK_SIGNAL_RETURN | SNUK_SIGNAL_CONTINUE, weak_ref);

        case SNUK_EXPR_CALL:
            return execute_call_expr(intpret, expr, weak_ref);

        case SNUK_EXPR_MEMBER:
            return execute_member_get(intpret, expr, weak_ref);

        case SNUK_EXPR_SELF: {
            SnukValue self_value = snuk_interpreter_get_env(intpret, self_str);
            SNUK_INTERPRETER_CHECK(intpret, self_value.type == SNUK_VALUE_TYPE_INST, "self error");
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

static SnukValue execute_assign_expr(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref) {
    SnukValue value = interpreter_eval_expr(intpret, expr->assign.value, weak_ref);
    SnukExpr *identifier = expr->assign.identifier;
    switch (identifier->type) {
        case SNUK_EXPR_IDENTIFIER:
            SNUK_INTERPRETER_CHECK(intpret, snuk_interpreter_set_env(intpret, identifier->identifier, value),
                                   "failed to set env value");
            break;

        case SNUK_EXPR_MEMBER: {
            SnukExpr *field = identifier->member_access.field;
            SnukValue type_or_inst = interpreter_eval_expr(intpret, identifier->member_access.type, weak_ref);
            SNUK_INTERPRETER_CHECK(
                intpret, interpreter_set_member(intpret, type_or_inst, field->identifier, value),
                "failed to set env value");
            interpreter_trash(intpret, type_or_inst);
            break;
        }

        default:
            SNUK_SHOULD_NOT_REACH_HERE;
            break;
    }
    return value;
}

static SnukValue execute_member_get(SnukInterpreter *intpret, SnukExpr *expr, bool weak_ref) {
    SnukValue type_or_inst = interpreter_eval_expr(intpret, expr->member_access.type, weak_ref);
    SnukValue res;
    if (type_or_inst.type == SNUK_VALUE_TYPE || type_or_inst.type == SNUK_VALUE_TYPE_INST) {
        res = interpreter_get_member(intpret, type_or_inst, expr->member_access.field->identifier);
    } else if (type_or_inst.type == SNUK_VALUE_NULL) {
        res = builtin_null_get_member(intpret, expr->member_access.field->identifier);
    } else {
        SnukExpr inst_expr = {
            .type = SNUK_EXPR_TYPE_INST,
            .type_inst_expr = {
                .type = NULL,
                .name = (SnukStringView){0},
                .init = snuk_darray_create(SnukExpr *, &intpret->allocator),
            },
        };

        SnukExpr value_expr;

        switch (type_or_inst.type) {
            case SNUK_VALUE_INT:
                inst_expr.type_inst_expr.type = &int_type;
                value_expr = (SnukExpr){
                    .type = SNUK_EXPR_INT,
                    .int_literal = type_or_inst.int_value,
                };
                break;
            case SNUK_VALUE_FLOAT:
                inst_expr.type_inst_expr.type = &float_type;
                value_expr = (SnukExpr){
                    .type = SNUK_EXPR_FLOAT,
                    .float_literal = type_or_inst.float_value,
                };
                break;
            case SNUK_VALUE_BOOL:
                inst_expr.type_inst_expr.type = &bool_type;
                value_expr = (SnukExpr){
                    .type = SNUK_EXPR_BOOL,
                    .bool_literal = type_or_inst.bool_value,
                };
                break;
            case SNUK_VALUE_STRING:
                inst_expr.type_inst_expr.type = &str_type;
                value_expr = (SnukExpr){
                    .type = SNUK_EXPR_STRING,
                    .string_literal = type_or_inst.string_value,
                };
                break;
            default:
                SNUK_SHOULD_NOT_REACH_HERE;
                break;
        }

        SnukExpr identifier_expr = {
            .type = SNUK_EXPR_IDENTIFIER,
            .identifier = value_str,
        };

        SnukExpr assign_expr = {
            .type = SNUK_EXPR_ASSIGN,
            .assign = {.identifier = &identifier_expr, .value = &value_expr},
        };

        snuk_darray_push(&inst_expr.type_inst_expr.init, &assign_expr);

        snuk_value_free(type_or_inst);
        type_or_inst = execute_inst_creation(intpret, &inst_expr, weak_ref);
        res = interpreter_get_member(intpret, type_or_inst, expr->member_access.field->identifier);
    }

    // insert the instance scope
    if (type_or_inst.type == SNUK_VALUE_TYPE_INST) {
        if (res.type == SNUK_VALUE_FN)
            res.fn_value.instance = snuk_ref_counter_retain_weak(type_or_inst.type_value.closure);
        else if (res.type == SNUK_VALUE_FN_NATIVE)
            res.native_fn.instance = snuk_ref_counter_retain_weak(type_or_inst.type_value.closure);
    }

    SNUK_INTERPRETER_CHECK(intpret, res.type != SNUK_VALUE_UNKOWN, "couldn't get the member");

    interpreter_trash(intpret, type_or_inst);
    return res;
}

static SnukValue execute_extend(SnukInterpreter *intpret, SnukItem *item, bool weak_ref) {
    SnukValue type = interpreter_eval_expr(intpret, item->extend_item.type, weak_ref);
    SNUK_INTERPRETER_CHECK(intpret, type.type == SNUK_VALUE_TYPE, "trying to extend non type");

    SnukRefCounter *temp = snuk_ref_counter_move(&intpret->current);
    intpret->current = snuk_ref_counter_move(&type.type_value.closure);

    uint64_t count = snuk_darray_get_length(item->extend_item.members);
    for (uint64_t i = 0; i < count; ++i) {
        SnukValue val = interpreter_exec_item(intpret, item->extend_item.members[i], true);
        SNUK_INTERPRETER_CHECK(intpret, intpret->signal == SNUK_SIGNAL_NONE, "signal is not none");
        snuk_value_free(val);
    }

    type.type_value.closure = snuk_ref_counter_move(&intpret->current);
    intpret->current = snuk_ref_counter_move(&temp);

    return type;
}

static SnukValue execute_interface(SnukInterpreter *intpret, SnukItem *item, bool weak_ref) {
    SNUK_UNUSED(weak_ref);
    SnukValue value = {
        .type = SNUK_VALUE_INTERFACE,
        .interface = {
            .type = item->interface_item.type,
        },
    };
    SNUK_INTERPRETER_CHECK(
        intpret,
        snuk_interpreter_create_env(intpret, item->interface_item.name, item->interface_item.type, value, false),
        "failed to create interface");
    return value;
}
