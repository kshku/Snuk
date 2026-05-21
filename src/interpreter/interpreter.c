#include "interpreter.h"

#include "io.h"
#include "parser/snuk_var.h"
#include "snuk_scope.h"

#include <stdio.h>

/**
 * @brief Walk the scope chain from current to global to resolve a name.
 */
SNUK_INLINE SnukEnv *interpreter_lookup(SnukInterpreter *intpret, SnukStringView name) {
    SnukRefCounter *rc = intpret->current;
    SnukEnv *env;
    while (rc) {
        if ((env = snuk_scope_lookup(rc, name))) return env;
        rc = GET_SCOPE_PARENT(rc);
    }
    return NULL;
}

/**
 * @brief Push a new child scope and make it the interpreter's current scope.
 */
SNUK_INLINE void interpreter_push_scope(SnukInterpreter *intpret) {
    intpret->current = snuk_scope_create(snuk_ref_counter_move(&intpret->current));
}

SNUK_INLINE SnukValue interpreter_get_self(SnukInterpreter *intpret) {
    SnukStringView self = {.str = "self", .len = 4};
    SnukValue self_value = snuk_interpreter_get_env(intpret, self);
    return self_value;
}

/**
 * @brief Pop the current scope and restore its parent as the active scope.
 */
SNUK_INLINE void interpreter_pop_scope(SnukInterpreter *intpret) {
    if (intpret->global == intpret->current) SNUK_SHOULD_NOT_REACH_HERE;

    SnukRefCounter *parent = snuk_ref_counter_retain(GET_SCOPE_PARENT(intpret->current));

    snuk_ref_counter_release(&intpret->current);
    intpret->current = snuk_ref_counter_move(&parent);
}

static SnukValue execute_unary_op(SnukInterpreter *intpret, SnukExpr *expr);
static SnukValue execute_binary_op(SnukInterpreter *intpret, SnukExpr *expr);
static SnukValue perform_binary_op(SnukValue left, SnukValue right, SnukTokenType op);
static void execute_print_expr(SnukInterpreter *intpret, SnukExpr **exprs);
static SnukValue execute_block_expr(
    SnukInterpreter *intpret, SnukExpr *block, int capture_signals, int propogate_signals);
static SnukValue execute_if_expr(SnukInterpreter *intpret, SnukExpr *expr);
static SnukValue execute_while_expr(SnukInterpreter *intpret, SnukExpr *loop);
static SnukValue execute_for_expr(SnukInterpreter *intpret, SnukExpr *loop);
static SnukValue execute_call_expr(SnukInterpreter *intpret, SnukExpr *expr);
static SnukValue execute_type_declaration(SnukInterpreter *intpret, SnukExpr *expr);
static SnukValue execute_inst_creation(SnukInterpreter *intpret, SnukExpr *expr);
static SnukValue execute_compound_assign(SnukInterpreter *intpret, SnukExpr *expr);
static SnukValue execute_member_access(SnukInterpreter *intpret, SnukExpr *expr);
static SnukValue execute_fn_expr(SnukInterpreter *intpret, SnukExpr *expr);

void snuk_interpreter_init(SnukInterpreter *intpret) {
    *intpret = (SnukInterpreter){
        .global = snuk_scope_create(NULL),
        .signal = SNUK_SIGNAL_NONE,
    };
    intpret->current = snuk_ref_counter_retain(intpret->global);
}

void snuk_interpreter_deinit(SnukInterpreter *intpret) {
    if (!intpret) return;

    SnukRefCounter *rc = intpret->current;
    while (rc) {
        SnukScope *scope = GET_SCOPE(rc);
        snuk_scope_destroy_envs(scope);
        rc = scope->parent;
    }

    snuk_ref_counter_release(&intpret->current);
    snuk_ref_counter_release(&intpret->global);

    *intpret = (SnukInterpreter){0};
}

SnukValue snuk_interpreter_get_env(SnukInterpreter *intpret, SnukStringView name) {
    SnukEnv *env = interpreter_lookup(intpret, name);
    if (!env) return (SnukValue){.type = SNUK_VALUE_UNKOWN};
    return snuk_value_copy(env->value);
}

bool snuk_interpreter_set_env(SnukInterpreter *intpret, SnukStringView name, SnukValue value) {
    SnukEnv *env = interpreter_lookup(intpret, name);
    SNUK_ASSERT(env, "var doesn't exists");
    SNUK_ASSERT(snuk_interpreter_value_is_of_type(intpret, value, env->type), "type mismatch");
    snuk_value_free(env->value);
    env->value = snuk_value_copy(value);
    return true;
}

bool snuk_interpreter_create_var(
    SnukInterpreter *intpret, SnukStringView name, SnukType *type, SnukValue value, bool is_const) {
    // TODO: constant
    SNUK_UNUSED(is_const);
    if (!snuk_interpreter_value_is_of_type(intpret, value, type)) return false;
    SnukEnv *env = snuk_env_create(name, type, value);
    if (!snuk_scope_add_env(intpret->current, env)) return false;
    return true;
}

SnukValue snuk_interpreter_exec_item(SnukInterpreter *intpret, SnukItem *item) {
    switch (item->type) {
        case SNUK_ITEM_EXPR:
            return snuk_interpreter_eval_expr(intpret, item->expr);

        case SNUK_ITEM_VAR_DECL:
        case SNUK_ITEM_CONST_DECL: {
            SnukValue value = snuk_interpreter_eval_expr(intpret, item->var->value);
            snuk_interpreter_create_var(
                intpret, item->var->name, item->var->type, value, item->type == SNUK_ITEM_CONST_DECL);
            return value;
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
            if (item->expr) value = snuk_interpreter_eval_expr(intpret, item->expr);
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

/**
 * @brief Evaluate an expression and return its runtime value.
 */
SnukValue snuk_interpreter_eval_expr(SnukInterpreter *intpret, SnukExpr *expr) {
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
            return execute_unary_op(intpret, expr);

        case SNUK_EXPR_BINARY:
            return execute_binary_op(intpret, expr);

        case SNUK_EXPR_ASSIGN: {
            SnukValue value = snuk_interpreter_eval_expr(intpret, expr->assign.value);
            snuk_interpreter_set_env(intpret, expr->assign.identifier->identifier, value);
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

        case SNUK_EXPR_FN:
            return execute_fn_expr(intpret, expr);

        case SNUK_EXPR_TYPE:
            return execute_type_declaration(intpret, expr);

        case SNUK_EXPR_TYPE_INST:
            return execute_inst_creation(intpret, expr);

        case SNUK_EXPR_BLOCK:
            return execute_block_expr(intpret, expr, SNUK_SIGNAL_BREAK, SNUK_SIGNAL_NONE);

        // TODO:
        case SNUK_EXPR_CALL:
            return execute_call_expr(intpret, expr);

        case SNUK_EXPR_MEMBER:
            return execute_member_access(intpret, expr);

        case SNUK_EXPR_SELF: {
            SnukValue self = interpreter_get_self(intpret);
            SNUK_ASSERT(self.type == SNUK_VALUE_TYPE_INST, "self error");
            return self;
        }

        case SNUK_EXPR_INDEX:
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

    snuk_value_free(val);
    return (SnukValue){.type = SNUK_VALUE_UNKOWN};
}

/**
 * @brief Apply a binary operator to two numeric values, promoting int to float
 * as needed.
 */
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

/**
 * @brief Evaluate both operands of a binary expression and combine them with
 * perform_binary_op.
 */
static SnukValue execute_binary_op(SnukInterpreter *intpret, SnukExpr *expr) {
    switch (expr->binary.op) {
        case SNUK_TOKEN_PIPE_PIPE:
        case SNUK_TOKEN_KW_OR: {
            SnukValue val = snuk_interpreter_eval_expr(intpret, expr->binary.left);
            bool bool_value = snuk_value_is_true(val);
            snuk_value_free(val);
            if (!bool_value) {
                val = snuk_interpreter_eval_expr(intpret, expr->binary.right);
                bool_value = snuk_value_is_true(val);
                snuk_value_free(val);
            }

            return (SnukValue){
                .type = SNUK_VALUE_BOOL,
                .bool_value = bool_value,
            };
        }

        case SNUK_TOKEN_AMP_AMP:
        case SNUK_TOKEN_KW_AND: {
            SnukValue val = snuk_interpreter_eval_expr(intpret, expr->binary.left);
            bool bool_value = snuk_value_is_true(val);
            snuk_value_free(val);
            if (bool_value) {
                val = snuk_interpreter_eval_expr(intpret, expr->binary.right);
                bool_value = snuk_value_is_true(val);
                snuk_value_free(val);
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
    snuk_value_free(left);
    snuk_value_free(right);

    // TODO: Errors, type checking
    return res;
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
            len = snuk_darray_get_length(value.fn_value.params);
            for (uint64_t i = 0; i < len; ++i) {
                if (i != 0) snuk_print(", ", NULL);
                snuk_print(SNUK_STRING_VIEW_FORMAT ": ",
                           SNUK_STRING_VIEW_ARG(value.fn_value.params[i]->name));
                interpreter_print_type(value.fn_value.params[i]->type);
            }
            snuk_print(") -> ");
            interpreter_print_type(value.fn_value.type->fn.return_type);
            break;
        case SNUK_VALUE_TYPE: {
            snuk_print("type{", NULL);
            SnukScope *scope = GET_SCOPE(value.type_value.closure);
            len = snuk_darray_get_length(scope->vars);
            for (uint64_t i = 0; i < len; ++i) {
                if (i != 0) snuk_print("; ", NULL);
                SnukEnv *env = scope->vars[i];
                snuk_print(SNUK_STRING_VIEW_FORMAT ": ", SNUK_STRING_VIEW_ARG(env->name));
                interpreter_print_type(env->type);
            }
            snuk_print("}", NULL);
            break;
        }
        case SNUK_VALUE_TYPE_INST: {
            snuk_print("type ", NULL);
            interpreter_print_type(value.type_value.type);
            snuk_print(" {", NULL);
            SnukScope *scope = GET_SCOPE(value.type_value.closure);
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
        }
        default:
            SNUK_SHOULD_NOT_REACH_HERE;
            break;
    }
}

/**
 * @brief Evaluate each expression in the darray and print its value to stdout.
 */
static void execute_print_expr(SnukInterpreter *intpret, SnukExpr **exprs) {
    if (!exprs) return;

    uint64_t count = snuk_darray_get_length(exprs);
    for (uint64_t i = 0; i < count; ++i) {
        SnukValue value = snuk_interpreter_eval_expr(intpret, exprs[i]);
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
    SnukInterpreter *intpret, SnukExpr *block, int capture_signals, int propogate_signals) {
    interpreter_push_scope(intpret);

    uint64_t count = snuk_darray_get_length(block->block_items);
    SnukValue value = {.type = SNUK_VALUE_NULL};

    for (uint64_t j = 0; j < count; ++j) {
        snuk_value_free(value);
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

    interpreter_pop_scope(intpret);
    return value;
}

/**
 * @brief Evaluate an if/else expression by selecting the matching branch block.
 */
static SnukValue execute_if_expr(SnukInterpreter *intpret, SnukExpr *expr) {
    SnukValue cond = snuk_interpreter_eval_expr(intpret, expr->if_else.condition);
    SnukValue res = {.type = SNUK_VALUE_NULL};

    if (snuk_value_is_true(cond)) {
        res = execute_block_expr(intpret, expr->if_else.then_block, SNUK_SIGNAL_NONE, SNUK_SIGNAL_ALL);
    } else if (expr->if_else.else_block) {
        if (expr->if_else.else_block->type == SNUK_EXPR_IF)
            res = execute_if_expr(intpret, expr->if_else.else_block);
        else
            res = execute_block_expr(intpret, expr->if_else.else_block, SNUK_SIGNAL_NONE, SNUK_SIGNAL_ALL);
    }

    snuk_value_free(cond);
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
        snuk_value_free(cond);
        cond = snuk_interpreter_eval_expr(intpret, loop->while_loop.condition);
        if (!snuk_value_is_true(cond)) goto end;
    }

    snuk_value_free(res);
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
        snuk_value_free(cond);
        cond = snuk_interpreter_eval_expr(intpret, loop->while_loop.condition);
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
static SnukValue execute_for_expr(SnukInterpreter *intpret, SnukExpr *loop) {
    interpreter_push_scope(intpret);
    SnukValue res = {.type = SNUK_VALUE_NULL};
    SnukValue cond = {.type = SNUK_VALUE_NULL};

    if (loop->for_loop.init) {
        SnukValue val = snuk_interpreter_exec_item(intpret, loop->for_loop.init);
        snuk_value_free(val);
    }

loop_start:
    if (loop->for_loop.condition) {
        snuk_value_free(cond);
        cond = snuk_interpreter_eval_expr(intpret, loop->for_loop.condition);
        if (!snuk_value_is_true(cond)) goto end;
    }

    snuk_value_free(res);
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
        snuk_value_free(val);
    }

    goto loop_start;

end:
    snuk_value_free(cond);
    interpreter_pop_scope(intpret);
    return res;
}

/**
 * @brief Test whether a parameter name appears in the function's parameter
 * list.
 */
SNUK_INLINE SnukType *get_type_from_param_list(SnukVar **params, uint64_t count, SnukStringView name) {
    for (uint64_t i = 0; i < count; ++i)
        if (snuk_string_view_equal(params[i]->name, name)) return params[i]->type;
    return NULL;
}

/**
 * @brief Bind call arguments to a function's parameters in a new scope and
 * execute its body.
 */
static SnukValue execute_call_expr(SnukInterpreter *intpret, SnukExpr *expr) {
    SnukValue fn = snuk_interpreter_eval_expr(intpret, expr->call.fn);
    SNUK_ASSERT(fn.type == SNUK_VALUE_FN, "call expression on non function");

    SnukRefCounter *new_scope = snuk_scope_create(snuk_ref_counter_retain(fn.fn_value.closure));

    uint64_t fn_param_count = snuk_darray_get_length(fn.fn_value.params);
    uint64_t call_param_count = snuk_darray_get_length(expr->call.params);
    bool named_params = false;
    for (uint64_t j = 0; j < call_param_count; ++j) {
        SnukExpr *call_param = expr->call.params[j];
        SnukVar *fn_param = fn.fn_value.params[j];

        SnukStringView name;
        SnukType *type;
        SnukExpr *value;
        // TODO:type
        if (call_param->type == SNUK_EXPR_ASSIGN) {
            named_params = true;
            name = call_param->assign.identifier->identifier;
            // TODO: parameter doesn't exists
            type = get_type_from_param_list(fn.fn_value.params, fn_param_count, name);
            SNUK_ASSERT(type, "parameter doesn't exists");
            value = call_param->assign.value;
        } else if (!named_params && call_param->type != SNUK_EXPR_COMPOUND_ASSIGN) {
            name = fn_param->name;
            type = fn_param->type;
            value = call_param;
        } else {
            // TODO: once named parameters are given, should not switch back to
            // positional
            SNUK_SHOULD_NOT_REACH_HERE;
        }
        SnukValue v = snuk_interpreter_eval_expr(intpret, value);
        SNUK_ASSERT(snuk_interpreter_value_is_of_type(intpret, v, type), "value type didn't match");
        SnukEnv *env = snuk_env_create(name, type, v);
        snuk_value_free(v);
        SNUK_ASSERT(snuk_scope_add_env(new_scope, env), "duplicate vars");
    }

    // check all parameters are filled, and if not fill with default value or
    // throw error
    for (uint64_t j = 0; j < fn_param_count; ++j) {
        SnukVar *param = fn.fn_value.params[j];
        if (!snuk_scope_lookup(new_scope, param->name)) {
            SNUK_ASSERT(param->value, "value is not given for parameter");
            SnukValue v = snuk_interpreter_eval_expr(intpret, param->value);
            SNUK_ASSERT(snuk_interpreter_value_is_of_type(intpret, v, param->type),
                        "value type "
                        "didn't match");
            SnukEnv *env = snuk_env_create(param->name, param->type, v);
            snuk_value_free(v);
            SNUK_ASSERT(snuk_scope_add_env(new_scope, env), "duplicate vars");
        }
    }

    SnukRefCounter *temp = snuk_ref_counter_move(&intpret->current);
    intpret->current = snuk_ref_counter_move(&new_scope);

    SnukValue ret = execute_block_expr(intpret, fn.fn_value.body, SNUK_SIGNAL_RETURN, SNUK_SIGNAL_NONE);

    new_scope = snuk_ref_counter_move(&intpret->current);
    intpret->current = snuk_ref_counter_move(&temp);
    snuk_ref_counter_release(&new_scope);

    snuk_value_free(fn);
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
            return item->var->name;

        default:
            SNUK_SHOULD_NOT_REACH_HERE;
            break;
    }
    return (SnukStringView){0};
}

/**
 * @brief Test whether a member name appear in the type's member list.
 */
SNUK_INLINE bool name_exists_in_member_list(SnukItem **members, uint64_t count, SnukStringView name) {
    for (uint64_t i = 0; i < count; ++i) {
        SnukStringView member_name = get_name_from_item(members[i]);
        if (snuk_string_view_equal(member_name, name)) return true;
    }

    return false;
}

static SnukValue execute_type_declaration(SnukInterpreter *intpret, SnukExpr *expr) {
    interpreter_push_scope(intpret);

    uint64_t count = snuk_darray_get_length(expr->type_expr.members);
    for (uint64_t i = 0; i < count; ++i) {
        SnukValue val = snuk_interpreter_exec_item(intpret, expr->type_expr.members[i]);
        snuk_value_free(val);
    }

    SnukValue value = {
        .type = SNUK_VALUE_TYPE,
        .type_value = {.type = expr->type_expr.type, .closure = snuk_ref_counter_retain(intpret->current)}
    };

    interpreter_pop_scope(intpret);

    // Syntax sugar
    if (expr->type_expr.name.len)
        snuk_interpreter_create_var(intpret, expr->type_expr.name, value.type_value.type, value, false);

    return value;
}

static SnukValue execute_inst_creation(SnukInterpreter *intpret, SnukExpr *expr) {
    SnukValue type = snuk_interpreter_get_env(intpret, expr->type_inst_expr.type->name);
    SNUK_ASSERT(type.type == SNUK_VALUE_TYPE, "type instance creation expression on non type");

    SnukRefCounter *new_scope = snuk_scope_create(snuk_ref_counter_retain(type.type_value.closure));

    SnukScope *type_scope = GET_SCOPE(type.type_value.closure);
    uint64_t member_count = snuk_darray_get_length(type_scope->vars);
    for (uint64_t i = 0; i < member_count; ++i) {
        SnukEnv *type_env = type_scope->vars[i];

        SnukValue value = snuk_value_copy(type_env->value);
        switch (value.type) {
            case SNUK_VALUE_FN:
                snuk_ref_counter_release(&value.fn_value.closure);
                value.fn_value.closure = snuk_ref_counter_retain(new_scope);
                break;

            case SNUK_VALUE_TYPE_INST:
            case SNUK_VALUE_TYPE:
                snuk_ref_counter_release(&value.type_value.closure);
                value.type_value.closure = snuk_ref_counter_retain(new_scope);
                break;

            default:
                break;
        }

        SnukEnv *env = snuk_env_create(type_env->name, type_env->type, value);
        SNUK_ASSERT(snuk_scope_add_env(new_scope, env), "duplicate vars");
        snuk_value_free(value);
    }

    uint64_t init_count = snuk_darray_get_length(expr->type_inst_expr.init);
    for (uint64_t i = 0; i < init_count; ++i) {
        SnukExpr *assign = expr->type_inst_expr.init[i];
        SNUK_ASSERT(assign->type == SNUK_EXPR_ASSIGN, "Expected assign expressions");
        SnukStringView name = assign->assign.identifier->identifier;
        SnukEnv *env = snuk_scope_lookup(new_scope, name);
        SNUK_ASSERT(env, "member doesn't exists");
        SnukValue v = snuk_interpreter_eval_expr(intpret, assign->assign.value);
        SNUK_ASSERT(snuk_interpreter_value_is_of_type(intpret, v, env->type),
                    "value type didn't "
                    "match");
        snuk_value_free(env->value);
        env->value = snuk_value_copy(v);
        snuk_value_free(v);
    }

    SnukValue value = {
        .type = SNUK_VALUE_TYPE_INST,
        .type_value = {
            .type = expr->type_inst_expr.type,
            .closure = snuk_ref_counter_move(&new_scope),
        },
    };

    snuk_value_free(type);

    // Syntax sugar
    if (expr->type_inst_expr.name.len) {
        SNUK_ASSERT(snuk_interpreter_value_is_of_type(intpret, value, value.type_value.type),
                    "value type didn't "
                    "match");
        SnukEnv *env = snuk_env_create(expr->type_inst_expr.name, value.type_value.type, value);
        SNUK_ASSERT(snuk_scope_add_env(intpret->current, env), "duplicate vars");
    }

    return value;
}

static SnukValue execute_compound_assign(SnukInterpreter *intpret, SnukExpr *expr) {
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

    SnukValue lhs = snuk_interpreter_eval_expr(intpret, expr->compound_assign.identifier);

    SnukValue rhs = snuk_interpreter_eval_expr(intpret, expr->compound_assign.value);

    SnukValue res = perform_binary_op(lhs, rhs, op_type);

    // TODO:
    if (!snuk_interpreter_set_env(intpret, expr->compound_assign.identifier->identifier, res))
        SNUK_SHOULD_NOT_REACH_HERE;

    snuk_value_free(lhs);
    snuk_value_free(rhs);

    return res;
}

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

bool snuk_interpreter_value_is_of_type(SnukInterpreter *intpret, SnukValue value, SnukType *type) {
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

static SnukValue execute_member_access(SnukInterpreter *intpret, SnukExpr *expr) {
    bool is_self = expr->member_access.type->type == SNUK_EXPR_SELF;
    SnukRefCounter *parent_of_type_inst = NULL;
    SnukStringView self = {.str = "self", .len = 4};

    SnukValue type = snuk_interpreter_eval_expr(intpret, expr->member_access.type);
    if (type.type == SNUK_VALUE_UNKOWN) SNUK_ASSERT(false, "something went wrong");
    SNUK_ASSERT(type.type == SNUK_VALUE_TYPE_INST || type.type == SNUK_VALUE_TYPE,
                "member access "
                "on non "
                "instance type");

    SnukRefCounter *temp = snuk_ref_counter_move(&intpret->current);
    intpret->current = snuk_ref_counter_retain(type.type_value.closure);

    // Add self
    if (!is_self) {
        SnukEnv *self_env = snuk_env_create(self, type.type_value.type, type);
        SNUK_ASSERT(snuk_scope_add_env(intpret->current, self_env), "self error");
        // Only access the scope of type instance
        SnukScope *scope = GET_SCOPE(type.type_value.closure);
        parent_of_type_inst = snuk_ref_counter_move(&scope->parent);
    }

    SnukValue ret = snuk_interpreter_eval_expr(intpret, expr->member_access.expr);

    // Remove self
    if (!is_self) {
        snuk_scope_remove_env(intpret->current, self);
        // Restore the parent of the instance
        SnukScope *scope = GET_SCOPE(type.type_value.closure);
        scope->parent = snuk_ref_counter_move(&parent_of_type_inst);
    }

    snuk_ref_counter_release(&intpret->current);
    intpret->current = snuk_ref_counter_move(&temp);

    snuk_value_free(type);
    return ret;
}

static SnukValue execute_fn_expr(SnukInterpreter *intpret, SnukExpr *expr) {
    interpreter_push_scope(intpret);

    // expr->fn_expr.params //darray

    interpreter_pop_scope(intpret);

    SnukValue value = {
                .type = SNUK_VALUE_FN,
                .fn_value = {
                    .closure = snuk_ref_counter_retain(intpret->current),
                    .body = expr->fn_expr.body,
                    .params = expr->fn_expr.params,
                    .type = expr->fn_expr.type,
                },
            };

    // Syntax sugar
    if (expr->fn_expr.name.len)
        snuk_interpreter_create_var(intpret, expr->fn_expr.name, value.fn_value.type, value, false);

    return value;
}
