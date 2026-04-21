#include "interpreter.h"

#include "memory.h"
#include <string.h>
#include "snuk_string.h"
#include "io.h"

static Value get_identifier_value(SnukInterpreter *i, SnukExpr *identifier);
static Value set_identifier_value(SnukInterpreter *i, SnukExpr *identifier, SnukExpr *value);

static Value get_unary_value(SnukInterpreter *i, SnukExpr *expr);
static Value get_binary_value(SnukInterpreter *i, SnukExpr *expr);
static Value perform_binary_op(Value left, Value right, SnukTokenType op);

static Value add_identifier(SnukInterpreter *i, SnukExpr *identifier, SnukExpr *expr);
static void print_exprs(SnukInterpreter *i, SnukExpr **exprs);

Value snuk_interpreter_exec_item(SnukInterpreter *i, SnukItem *item) {
    switch (item->type) {
        case SNUK_ITEM_EXPR:
            return snuk_interpreter_eval_expr(i, item->expr);
            break;
        case SNUK_ITEM_VAR_DECL:
        case SNUK_ITEM_CONST_DECL:
            // TODO: const
            return add_identifier(i, item->var_decl.identifier, item->var_decl.init);
            break;

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
            return (Value){.type = VALUE_NULL};
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
    return (Value){.type = VALUE_UNKOWN};
}

Value snuk_interpreter_eval_expr(SnukInterpreter *i, SnukExpr *expr) {
    switch (expr->type) {
        case SNUK_EXPR_IDENTIFIER:
            return get_identifier_value(i, expr);

        case SNUK_EXPR_INT:
            return (Value){
                .type = VALUE_INT,
                .int_value = expr->int_literal,
            };

        case SNUK_EXPR_FLOAT:
            return (Value){
                .type = VALUE_FLOAT,
                .float_value = expr->float_literal,
            };

        case SNUK_EXPR_STRING:
            return (Value){
                .type = VALUE_STRING, 
                .string_value = snuk_string_view_copy(expr->string_literal),
            };

        case SNUK_EXPR_BOOL:
            return (Value){
                .type = VALUE_BOOL,
                .bool_value = expr->bool_literal,
            };

        case SNUK_EXPR_NULL:
            return (Value){
                .type = VALUE_NULL,
            };

        case SNUK_EXPR_UNARY:
            return get_unary_value(i, expr);

        case SNUK_EXPR_BINARY:
            return get_binary_value(i, expr);

        case SNUK_EXPR_ASSIGN:
            return set_identifier_value(i, expr->assign.identifier, expr->assign.value);

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

    return (Value){.type = VALUE_UNKOWN};
}

static Value get_identifier_value(SnukInterpreter *i, SnukExpr *identifier) {
    SNUK_ASSERT(identifier->identifier.len > 0, "identifier name is empty");

    uint64_t index = (uint64_t)identifier->identifier.str[0];
    uint64_t length = snuk_darray_get_length(i->envs);

    if (length < index) goto fail;
    if (!i->envs[index]) goto fail;

    uint64_t count = snuk_darray_get_length(i->envs[index]);
    for (uint64_t j = 0; j < count; ++j) {
        if (identifier->identifier.len != i->envs[index][j].identifier.len) continue;
        if (string_n_equal(identifier->identifier.str,
                    i->envs[index][j].identifier.str, identifier->identifier.len)) {
            return i->envs[index][j].value;
        }
    }

fail:
    return (Value){.type = VALUE_UNKOWN};
}

static Value set_identifier_value(SnukInterpreter *i, SnukExpr *identifier, SnukExpr *expr) {
    SNUK_ASSERT(identifier->identifier.len > 0, "identifier name is empty");

    Value value = snuk_interpreter_eval_expr(i, expr);
    uint64_t index = (uint64_t)identifier->identifier.str[0];
    uint64_t length = snuk_darray_get_length(i->envs);

    if (length < index) goto fail;
    if (!i->envs[index]) goto fail;

    uint64_t count = snuk_darray_get_length(i->envs[index]);
    for (uint64_t j = 0; j < count; ++j) {
        if (identifier->identifier.len != i->envs[index][j].identifier.len) continue;
        if (string_n_equal(identifier->identifier.str,
                    i->envs[index][j].identifier.str, identifier->identifier.len)) {
            i->envs[index][j].value = value;
            return value;
        }
    }

    // TODO: errors
fail:
    SNUK_SHOULD_NOT_REACH_HERE;
    return (Value){.type = VALUE_UNKOWN};
}

static Value get_unary_value(SnukInterpreter *i, SnukExpr *expr) {
    Value val = snuk_interpreter_eval_expr(i, expr->unary.operand);

    switch (expr->unary.op) {
        case SNUK_TOKEN_PLUS:
        case SNUK_TOKEN_MINUS:
            switch (val.type) {
                case VALUE_INT:
                    if (expr->unary.op == SNUK_TOKEN_MINUS)
                        val.int_value *= -1;
                    break;
                case VALUE_FLOAT:
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

    return (Value){.type = VALUE_UNKOWN};
}

static Value perform_binary_op(Value left, Value right, SnukTokenType op) {
    if (left.type != VALUE_INT && left.type != VALUE_FLOAT) goto fail;
    if (right.type != VALUE_INT && right.type != VALUE_FLOAT) goto fail;

    Value res = {0};

    if (left.type == VALUE_FLOAT || right.type == VALUE_FLOAT) res.type = VALUE_FLOAT;
    else res.type = VALUE_INT;

    // TODO: better way to do this
    switch (op) {
        case SNUK_TOKEN_PLUS:
            if (res.type == VALUE_INT) {
                res.int_value = left.int_value + right.int_value;
            } else {
                if (left.type == VALUE_INT)
                    res.float_value = (double)left.int_value + right.float_value;
                else if (right.type == VALUE_INT)
                    res.float_value = left.float_value + (double)right.int_value;
                else
                    res.float_value = left.float_value + right.float_value;
            }
            return res;

        case SNUK_TOKEN_MINUS:
            if (res.type == VALUE_INT) {
                res.int_value = left.int_value - right.int_value;
            } else {
                if (left.type == VALUE_INT)
                    res.float_value = (double)left.int_value - right.float_value;
                else if (right.type == VALUE_INT)
                    res.float_value = left.float_value - (double)right.int_value;
                else
                    res.float_value = left.float_value - right.float_value;
            }
            return res;

        case SNUK_TOKEN_STAR:
            if (res.type == VALUE_INT) {
                res.int_value = left.int_value * right.int_value;
            } else {
                if (left.type == VALUE_INT)
                    res.float_value = (double)left.int_value * right.float_value;
                else if (right.type == VALUE_INT)
                    res.float_value = left.float_value * (double)right.int_value;
                else
                    res.float_value = left.float_value * right.float_value;
            }
            return res;

        case SNUK_TOKEN_SLASH:
            if (res.type == VALUE_INT) {
                res.int_value = left.int_value / right.int_value;
            } else {
                if (left.type == VALUE_INT)
                    res.float_value = (double)left.int_value / right.float_value;
                else if (right.type == VALUE_INT)
                    res.float_value = left.float_value / (double)right.int_value;
                else
                    res.float_value = left.float_value / right.float_value;
            }
            return res;

        case SNUK_TOKEN_PERCENT:
            if (res.type != VALUE_INT) goto fail;
            res.int_value = left.int_value % right.int_value;
            return res;

            // TODO: more binary ops

        default:
            break;
    }

fail:
    return (Value){.type = VALUE_UNKOWN};
}

static Value get_binary_value(SnukInterpreter *i, SnukExpr *expr) {
    Value left = snuk_interpreter_eval_expr(i, expr->binary.left);
    Value right = snuk_interpreter_eval_expr(i, expr->binary.right);
    
    // TODO: Errors, type checking
    return perform_binary_op(left, right, expr->binary.op);
}

static Value add_identifier(SnukInterpreter *i, SnukExpr *identifier, SnukExpr *expr) {
    // TODO: multiple declaration errors

    Value value = snuk_interpreter_eval_expr(i, expr);
    uint64_t index = (uint64_t)identifier->identifier.str[0];
    uint64_t length = snuk_darray_get_length(i->envs);

    if (length < index)
        snuk_darray_push_at(&i->envs, index, snuk_darray_create(Env));
    else if (!i->envs[index])
        i->envs[index] = snuk_darray_create(Env);

    Env env = {
        .identifier = snuk_string_view_copy(identifier->identifier),
        .value = value,
    };
    snuk_darray_push(&i->envs[index], env);

    return value;
}

static void print_exprs(SnukInterpreter *i, SnukExpr **exprs) {
    if (!exprs) return;

    uint64_t count = snuk_darray_get_length(exprs);
    for (uint64_t j = 0; j < count; ++j) {
        snuk_interpreter_print_value(snuk_interpreter_eval_expr(i, exprs[j]));
    }

    snuk_darray_destroy(exprs);
}

void snuk_interpreter_print_value(Value value) {
    switch (value.type) {
        case VALUE_UNKOWN:
            snuk_println("type: %s", SNUK_STRINGIFY(VALUE_UNKOWN));
            break;
        case VALUE_INT:
            snuk_println("type: %s", SNUK_STRINGIFY(VALUE_INT));
            snuk_println("value: %ld", value.int_value);
            break;
        case VALUE_FLOAT:
            snuk_println("type: %s", SNUK_STRINGIFY(VALUE_FLOAT));
            snuk_println("value: %lf", value.float_value);
            break;
        case VALUE_BOOL:
            snuk_println("type: %s", SNUK_STRINGIFY(VALUE_BOOL));
            snuk_println("value: %s", value.bool_value ? "true" : "false");
            break;
        case VALUE_STRING:
            snuk_println("type: %s", SNUK_STRINGIFY(VALUE_STRING));
            snuk_println("value: "SNUK_STRING_VIEW_FORMAT, SNUK_STRING_VIEW_ARG(value.string_value));
            break;
        case VALUE_NULL:
            snuk_println("type: %s", SNUK_STRINGIFY(VALUE_NULL));
            break;
        case VALUE_FN:
            snuk_println("type: %s", SNUK_STRINGIFY(VALUE_FN));
            break;
        case VALUE_TYPE:
            snuk_println("type: %s", SNUK_STRINGIFY(VALUE_TYPE));
            break;
        default:
            SNUK_SHOULD_NOT_REACH_HERE;
            break;
    }
}

