#include "interpreter.h"

#include "memory.h"
#include <string.h>
#include "snuk_string.h"
#include "io.h"

SNUK_INLINE SnukEnv *create_snuk_env(SnukInterpreter *i, SnukStringView name, SnukExpr *value) {
    SnukEnv *env = (SnukEnv *)snuk_alloc(sizeof(SnukEnv), alignof(SnukEnv));
    *env = (SnukEnv){
        .name = snuk_string_view_copy(name),
        .value = snuk_interpreter_eval_expr(i, value),
    };
    return env;
}

static void snuk_scope_push(SnukInterpreter *i);
static void snuk_scope_pop(SnukInterpreter *i);
static SnukEnv *snuk_scope_add_env(SnukScope *scope, SnukEnv *env);
static SnukEnv *snuk_scope_lookup(SnukScope *scope, SnukStringView name);
static SnukEnv *snuk_env_lookup(SnukInterpreter *i, SnukStringView name);

static SnukValue get_unary_value(SnukInterpreter *i, SnukExpr *expr);
static SnukValue get_binary_value(SnukInterpreter *i, SnukExpr *expr);
static SnukValue perform_binary_op(SnukValue left, SnukValue right, SnukTokenType op);

static void print_exprs(SnukInterpreter *i, SnukExpr **exprs);

SnukValue snuk_interpreter_exec_item(SnukInterpreter *i, SnukItem *item) {
    // TODO: avoiding unused function warnings
    snuk_scope_push(i);
    snuk_scope_pop(i);

    switch (item->type) {
        case SNUK_ITEM_EXPR:
            return snuk_interpreter_eval_expr(i, item->expr);
            break;
        case SNUK_ITEM_VAR_DECL:
        case SNUK_ITEM_CONST_DECL:
            {
                // TODO: const
                SnukEnv *env = create_snuk_env(i, item->var_decl.identifier->identifier, item->var_decl.init);
                return snuk_scope_add_env(i->current, env)->value;
            }
        // TODO:
        case SNUK_ITEM_RETURN:
            break;
        case SNUK_ITEM_BREAK:
            break;
        case SNUK_ITEM_CONTINUE:
            break;
        case SNUK_ITEM_FN_DECL:
            break;
        case SNUK_ITEM_TYPE_DECL:
            break;

        case SNUK_ITEM_PRINT:
            print_exprs(i, item->print_exprs);
            // TODO: return something else?
            return (SnukValue){.type = SNUK_VALUE_NULL};
            break;

        // TODO:
        case SNUK_ITEM_LINE_COMMENT:
            break;
        case SNUK_ITEM_BLOCK_COMMENT:
            break;
        case SNUK_ITEM_MAX:
        default:
            break;
    }
    return (SnukValue){.type = SNUK_VALUE_UNKOWN};
}

SnukValue snuk_interpreter_eval_expr(SnukInterpreter *i, SnukExpr *expr) {
    switch (expr->type) {
        case SNUK_EXPR_IDENTIFIER:
            return snuk_env_lookup(i, expr->identifier)->value;

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
                .string_value = snuk_string_view_copy(expr->string_literal),
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
            return get_unary_value(i, expr);

        case SNUK_EXPR_BINARY:
            return get_binary_value(i, expr);

        case SNUK_EXPR_ASSIGN:
            {
                SnukValue value = snuk_interpreter_eval_expr(i, expr->assign.value);
                snuk_env_lookup(i, expr->assign.identifier->identifier)->value = value;
                return value;
            }

        // TODO:
        case SNUK_EXPR_CALL:
            break;
        case SNUK_EXPR_MEMBER:
            break;
        case SNUK_EXPR_INDEX:
            break;
        case SNUK_EXPR_MAX:
        default:
            break;
    }

    return (SnukValue){.type = SNUK_VALUE_UNKOWN};
}

static SnukValue get_unary_value(SnukInterpreter *i, SnukExpr *expr) {
    SnukValue val = snuk_interpreter_eval_expr(i, expr->unary.operand);

    switch (expr->unary.op) {
        case SNUK_TOKEN_PLUS:
        case SNUK_TOKEN_MINUS:
            switch (val.type) {
                case SNUK_VALUE_INT:
                    if (expr->unary.op == SNUK_TOKEN_MINUS)
                        val.int_value *= -1;
                    break;
                case SNUK_VALUE_FLOAT:
                    if (expr->unary.op == SNUK_TOKEN_MINUS)
                        val.float_value *= -1;
                    break;
                default:
                    // TODO: Errors
                    break;
            }
            return val;

            // TODO: more unary ops
            
        default:
            break;
    }

    return (SnukValue){.type = SNUK_VALUE_UNKOWN};
}

static SnukValue perform_binary_op(SnukValue left, SnukValue right, SnukTokenType op) {
    if (left.type != SNUK_VALUE_INT && left.type != SNUK_VALUE_FLOAT) goto fail;
    if (right.type != SNUK_VALUE_INT && right.type != SNUK_VALUE_FLOAT) goto fail;

    SnukValue res = {0};

    if (left.type == SNUK_VALUE_FLOAT || right.type == SNUK_VALUE_FLOAT) res.type = SNUK_VALUE_FLOAT;
    else res.type = SNUK_VALUE_INT;

    // TODO: better way to do this
    switch (op) {
        case SNUK_TOKEN_PLUS:
            if (res.type == SNUK_VALUE_INT) {
                res.int_value = left.int_value + right.int_value;
            } else {
                if (left.type == SNUK_VALUE_INT)
                    res.float_value = (double)left.int_value + right.float_value;
                else if (right.type == SNUK_VALUE_INT)
                    res.float_value = left.float_value + (double)right.int_value;
                else
                    res.float_value = left.float_value + right.float_value;
            }
            return res;

        case SNUK_TOKEN_MINUS:
            if (res.type == SNUK_VALUE_INT) {
                res.int_value = left.int_value - right.int_value;
            } else {
                if (left.type == SNUK_VALUE_INT)
                    res.float_value = (double)left.int_value - right.float_value;
                else if (right.type == SNUK_VALUE_INT)
                    res.float_value = left.float_value - (double)right.int_value;
                else
                    res.float_value = left.float_value - right.float_value;
            }
            return res;

        case SNUK_TOKEN_STAR:
            if (res.type == SNUK_VALUE_INT) {
                res.int_value = left.int_value * right.int_value;
            } else {
                if (left.type == SNUK_VALUE_INT)
                    res.float_value = (double)left.int_value * right.float_value;
                else if (right.type == SNUK_VALUE_INT)
                    res.float_value = left.float_value * (double)right.int_value;
                else
                    res.float_value = left.float_value * right.float_value;
            }
            return res;

        case SNUK_TOKEN_SLASH:
            if (res.type == SNUK_VALUE_INT) {
                res.int_value = left.int_value / right.int_value;
            } else {
                if (left.type == SNUK_VALUE_INT)
                    res.float_value = (double)left.int_value / right.float_value;
                else if (right.type == SNUK_VALUE_INT)
                    res.float_value = left.float_value / (double)right.int_value;
                else
                    res.float_value = left.float_value / right.float_value;
            }
            return res;

        case SNUK_TOKEN_PERCENT:
            if (res.type != SNUK_VALUE_INT) goto fail;
            res.int_value = left.int_value % right.int_value;
            return res;

            // TODO: more binary ops

        default:
            break;
    }

fail:
    return (SnukValue){.type = SNUK_VALUE_UNKOWN};
}

static SnukValue get_binary_value(SnukInterpreter *i, SnukExpr *expr) {
    SnukValue left = snuk_interpreter_eval_expr(i, expr->binary.left);
    SnukValue right = snuk_interpreter_eval_expr(i, expr->binary.right);
    
    // TODO: Errors, type checking
    return perform_binary_op(left, right, expr->binary.op);
}

static void print_exprs(SnukInterpreter *i, SnukExpr **exprs) {
    if (!exprs) return;

    uint64_t count = snuk_darray_get_length(exprs);
    for (uint64_t j = 0; j < count; ++j) {
        snuk_interpreter_print_value(snuk_interpreter_eval_expr(i, exprs[j]));
    }

    snuk_darray_destroy(exprs);
}

void snuk_interpreter_print_value(SnukValue value) {
    switch (value.type) {
        case SNUK_VALUE_UNKOWN:
            snuk_println("type: %s", SNUK_STRINGIFY(SNUK_VALUE_UNKOWN));
            break;
        case SNUK_VALUE_INT:
            snuk_println("type: %s", SNUK_STRINGIFY(SNUK_VALUE_INT));
            snuk_println("value: %ld", value.int_value);
            break;
        case SNUK_VALUE_FLOAT:
            snuk_println("type: %s", SNUK_STRINGIFY(SNUK_VALUE_FLOAT));
            snuk_println("value: %lf", value.float_value);
            break;
        case SNUK_VALUE_BOOL:
            snuk_println("type: %s", SNUK_STRINGIFY(SNUK_VALUE_BOOL));
            snuk_println("value: %s", value.bool_value ? "true" : "false");
            break;
        case SNUK_VALUE_STRING:
            snuk_println("type: %s", SNUK_STRINGIFY(SNUK_VALUE_STRING));
            snuk_println("value: "SNUK_STRING_VIEW_FORMAT, SNUK_STRING_VIEW_ARG(value.string_value));
            break;
        case SNUK_VALUE_NULL:
            snuk_println("type: %s", SNUK_STRINGIFY(SNUK_VALUE_NULL));
            break;
        case SNUK_VALUE_FN:
            snuk_println("type: %s", SNUK_STRINGIFY(SNUK_VALUE_FN));
            break;
        case SNUK_VALUE_TYPE:
            snuk_println("type: %s", SNUK_STRINGIFY(SNUK_VALUE_TYPE));
            break;
        default:
            SNUK_SHOULD_NOT_REACH_HERE;
            break;
    }
}

static void snuk_scope_push(SnukInterpreter *i) {
    SnukScope *scope = (SnukScope *)snuk_alloc(sizeof(SnukScope), alignof(SnukScope));
    *scope = (SnukScope){
        .vars = snuk_darray_create(SnukEnv *),
        .parent = i->current,
    };
    i->current = scope;
}

static void snuk_scope_pop(SnukInterpreter *i) {
    if (i->global == i->current) SNUK_SHOULD_NOT_REACH_HERE;
    SnukScope *scope = i->current;
    i->current = i->current->parent;
    snuk_darray_destroy(scope->vars);
    snuk_free(scope);
}

static SnukEnv *snuk_scope_add_env(SnukScope *scope, SnukEnv *env) {
    // TODO: multiple declaration errors

    snuk_darray_push(&scope->vars, env);

    return env;
}

static SnukEnv *snuk_scope_lookup(SnukScope *scope, SnukStringView name) {
    uint64_t count = snuk_darray_get_length(scope->vars);
    for (uint64_t j = 0; j < count; ++j) {
        if (name.len != scope->vars[j]->name.len) continue;
        if (string_n_equal(name.str, scope->vars[j]->name.str, name.len))
            return scope->vars[j];
    }
    return NULL;
}

static SnukEnv *snuk_env_lookup(SnukInterpreter *i, SnukStringView name) {
    SnukScope *scope = i->current;
    SnukEnv *env;
    while (scope) {
        if ((env = snuk_scope_lookup(scope, name))) return env;
        scope = scope->parent;
    }
    return NULL;
}
