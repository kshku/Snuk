#include "interpreter.h"

#include "memory.h"
#include <string.h>
#include "snuk_string.h"
#include "io.h"

SNUK_INLINE char *copy_string(const char *str, uint64_t length) {
    char *copy = snuk_alloc(sizeof(char) * (length + 1), alignof(char));
    memcpy(copy, str, length);
    copy[length] = 0;
    return copy;
}

SNUK_INLINE SnukIdentifier copy_identifier(SnukExpr *identifier) {
    SNUK_ASSERT(identifier->type == SNUK_EXPR_IDENTIFIER, "not identifier");
    SnukIdentifier ident = {
        .name = copy_string(identifier->identifier.name, identifier->identifier.length),
        .length = identifier->identifier.length,
    };
    return ident;
}

static Value get_identifier_value(SnukInterpreter *i, SnukExpr *identifier);
static Value set_identifier_value(SnukInterpreter *i, SnukExpr *identifier, SnukExpr *value);

static Value get_unary_value(SnukInterpreter *i, SnukExpr *expr);
static Value get_binary_value(SnukInterpreter *i, SnukExpr *expr);
static Value perform_binary_op(Value left, Value right, SnukTokenType op);

static void add_identifier(SnukInterpreter *i, SnukExpr *identifier, SnukExpr *expr);
static void print_exprs(SnukInterpreter *i, SnukExpr **exprs);

void snuk_interpreter_exec_stmt(SnukInterpreter *i, SnukStmt *stmt) {
    switch (stmt->type) {
        case SNUK_STMT_EXPR:
            snuk_interpreter_print_value(snuk_interpreter_eval_expr(i, stmt->expr_stmt));
            break;
        case SNUK_STMT_VAR_DECL:
        case SNUK_STMT_CONST_DECL:
            // TODO: const
            add_identifier(i, stmt->decl_stmt.identifier, stmt->decl_stmt.init);
            break;

        // TODO:
        case SNUK_STMT_IF:
            break;
        case SNUK_STMT_MATCH:
            break;
        case SNUK_STMT_WHILE:
            break;
        case SNUK_STMT_DO_WHILE:
            break;
        case SNUK_STMT_FOR:
            break;
        case SNUK_STMT_RETURN:
            break;
        case SNUK_STMT_BREAK:
            break;
        case SNUK_STMT_CONTINUE:
            break;
        case SNUK_STMT_FN:
            break;
        case SNUK_STMT_TYPE:
            break;

        case SNUK_STMT_PRINT:
            print_exprs(i, stmt->print_stmt.exprs);
            break;

        // TODO:
        case SNUK_STMT_BLOCK:
            break;
        case SNUK_STMT_SLCOMMENT:
            break;
        case SNUK_STMT_MLCOMMENT:
            break;
        case SNUK_STMT_MAX:
        default:
            break;
    }
}

Value snuk_interpreter_eval_expr(SnukInterpreter *i, SnukExpr *expr) {
    switch (expr->type) {
        case SNUK_EXPR_IDENTIFIER:
            return get_identifier_value(i, expr);

        case SNUK_EXPR_INT_LITERAL:
            return (Value){
                .type = VALUE_INT,
                .int_value = expr->int_literal,
            };

        case SNUK_EXPR_FLOAT_LITERAL:
            return (Value){
                .type = VALUE_FLOAT,
                .int_value = expr->float_literal,
            };

        case SNUK_EXPR_STRING_LITERAL:
            return (Value){
                .type = VALUE_STRING, 
                .string_value = {
                    .string = copy_string(expr->string_literal.value, expr->string_literal.length),
                    .length = expr->string_literal.length,
                },
            };

        case SNUK_EXPR_TRUE_LITERAL:
        case SNUK_EXPR_FALSE_LITERAL:
            return (Value){
                .type = VALUE_BOOL,
                .bool_value = expr->type == SNUK_EXPR_TRUE_LITERAL,
            };

        case SNUK_EXPR_NULL_LITERAL:
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
    SNUK_ASSERT(identifier->identifier.length > 0, "identifier name is empty");

    uint64_t index = (uint64_t)identifier->identifier.name[0];
    uint64_t length = snuk_darray_get_length(i->envs);

    if (length < index) goto fail;
    if (!i->envs[index]) goto fail;

    uint64_t count = snuk_darray_get_length(i->envs[index]);
    for (uint64_t j = 0; j < count; ++j) {
        if (identifier->identifier.length != i->envs[index][j].identifier.length) continue;
        if (string_n_equal(identifier->identifier.name,
                    i->envs[index][j].identifier.name, identifier->identifier.length)) {
            return i->envs[index][j].value;
        }
    }

fail:
    return (Value){.type = VALUE_UNKOWN};
}

static Value set_identifier_value(SnukInterpreter *i, SnukExpr *identifier, SnukExpr *expr) {
    SNUK_ASSERT(identifier->identifier.length > 0, "identifier name is empty");

    Value value = snuk_interpreter_eval_expr(i, expr);
    uint64_t index = (uint64_t)identifier->identifier.name[0];
    uint64_t length = snuk_darray_get_length(i->envs);

    if (length < index) goto fail;
    if (!i->envs[index]) goto fail;

    uint64_t count = snuk_darray_get_length(i->envs[index]);
    for (uint64_t j = 0; j < count; ++j) {
        if (identifier->identifier.length != i->envs[index][j].identifier.length) continue;
        if (string_n_equal(identifier->identifier.name,
                    i->envs[index][j].identifier.name, identifier->identifier.length)) {
            i->envs[index][j].value = value;
            return value;
        }
    }

    // TODO: errors
fail:
    SNUK_SHOULD_NOT_REACH_HERE;
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

static void add_identifier(SnukInterpreter *i, SnukExpr *identifier, SnukExpr *expr) {
    // TODO: multiple declaration errors

    Value value = snuk_interpreter_eval_expr(i, expr);
    uint64_t index = (uint64_t)identifier->identifier.name[0];
    uint64_t length = snuk_darray_get_length(i->envs);

    if (length < index)
        snuk_darray_push_at(&i->envs, index, snuk_darray_create(Env));
    else if (!i->envs[index])
        i->envs[index] = snuk_darray_create(Env);

    Env env = {
        .identifier = copy_identifier(identifier),
        .value = value,
    };
    snuk_darray_push(&i->envs[index], env);
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
            snuk_println("value: %.*s", value.string_value.length, value.string_value.string);
            break;
        case VALUE_NULL:
            snuk_println("type: %s", SNUK_STRINGIFY(VALUE_NULL));
            break;
        default:
            SNUK_SHOULD_NOT_REACH_HERE;
            break;
    }
}

