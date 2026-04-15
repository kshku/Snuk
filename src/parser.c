#include "parser.h"

#include "parser_helper.h"

#include "io.h"
#include "memory.h"
#include "logger.h"

SnukStmt *snuk_parser_next_stmt(SnukParser *parser) {
    if (parser->current.type == SNUK_TOKEN_EOF) return NULL;
    SnukStmt *stmt = parse_stmt(parser);
    if (parser->panic_mode) parser_sync(parser);
    return stmt;
}

static SnukStmt *parse_stmt(SnukParser *parser) {
    if (parser_match(parser, SNUK_TOKEN_VAR) || parser_match(parser, SNUK_TOKEN_CONST))
        return parse_decl_stmt(parser, parser->previous.type == SNUK_TOKEN_CONST);

    if (parser_match(parser, SNUK_TOKEN_IF)) return parse_if_stmt(parser);

    if (parser_match(parser, SNUK_TOKEN_MATCH)) return parse_match_stmt(parser);

    if (parser_match(parser, SNUK_TOKEN_WHILE)) return parse_while_stmt(parser);
    if (parser_match(parser, SNUK_TOKEN_DO)) return parse_do_while_stmt(parser);
    if (parser_match(parser, SNUK_TOKEN_FOR)) return parse_for_stmt(parser);

    if (parser_match(parser, SNUK_TOKEN_RETURN) || parser_match(parser, SNUK_TOKEN_CONTINUE)
            || parser_match(parser, SNUK_TOKEN_BREAK))
        return parse_flow_stmt(parser);

    if (parser_match(parser, SNUK_TOKEN_FN)) return parse_fn_stmt(parser);

    if (parser_match(parser, SNUK_TOKEN_TYPE)) return parse_type_stmt(parser);

    if (parser_match(parser, SNUK_TOKEN_PRINT)) return parse_print_stmt(parser);

    if (parser_match(parser, SNUK_TOKEN_LBRACE)) return parse_block_stmt(parser);

    if (parser_match(parser, SNUK_TOKEN_MLCOMMENT) || parser_match(parser, SNUK_TOKEN_SLCOMMENT))
        return parse_comment_stmt(parser);

    return parse_expr_stmt(parser);
}

static SnukStmt *parse_expr_stmt(SnukParser *parser) {
    return build_expr_stmt(parser, parse_expression(parser));
}

static SnukStmt *parse_decl_stmt(SnukParser *parser, bool is_const) {
    parser_expect(parser, SNUK_TOKEN_IDENTIFIER, "expected an identifier");
    SnukToken identifier = parser->previous;

    if (parser_match(parser, SNUK_TOKEN_COLON)) {
        // TODO: types
        parser_expect(parser, SNUK_TOKEN_IDENTIFIER, "expected a type");
    }

    SnukExpr *init = NULL;
    if (parser_match(parser, SNUK_TOKEN_ASSIGN)) init = parse_expression(parser);
    else init = build_null_expr(parser);

    return build_decl_stmt(parser, identifier, init, is_const);
}

static SnukStmt *parse_if_stmt(SnukParser *parser) {
    SnukExpr *condition = parse_expression(parser);
    parser_expect(parser, SNUK_TOKEN_LBRACE, "expected '{'");
    SnukStmt *then_branch = parse_block_stmt(parser);
    SnukStmt *else_branch = NULL;
    if (parser_match(parser, SNUK_TOKEN_ELSE)) {
        if (parser_match(parser, SNUK_TOKEN_IF)) {
            else_branch = parse_if_stmt(parser);
        } else {
            parser_expect(parser, SNUK_TOKEN_LBRACE, "expected '{'");
            else_branch = parse_block_stmt(parser);
        }
    }
    return build_if_stmt(parser, condition, then_branch, else_branch);
}

static SnukStmt *parse_match_stmt(SnukParser *parser) {
    SNUK_UNUSED(parser);
    // TODO:
    return NULL;
}

static SnukStmt *parse_while_stmt(SnukParser *parser) {
    SnukExpr *condition = parse_expression(parser);
    parser_expect(parser, SNUK_TOKEN_LBRACE, "expected '{'");
    SnukStmt *block = parse_block_stmt(parser);
    return build_while_stmt(parser, condition, block, false);
}

static SnukStmt *parse_do_while_stmt(SnukParser *parser) {
    parser_expect(parser, SNUK_TOKEN_LBRACE, "expected '{'");
    SnukStmt *block = parse_block_stmt(parser);
    parser_expect(parser, SNUK_TOKEN_WHILE, "expected while condition");
    SnukExpr *condition = parse_expression(parser);
    return build_while_stmt(parser, condition, block, true);
}

static SnukStmt *parse_for_stmt(SnukParser *parser) {
    SnukStmt *init = NULL;
    SnukExpr *cond = NULL;
    SnukExpr *update = NULL;
    SnukStmt *block = NULL;

    if (parser_match(parser, SNUK_TOKEN_LBRACE)) {
        // infinite loop
        block = parse_block_stmt(parser);
        return build_for_stmt(parser, init, cond, update, block);
    }

    if (parser_match(parser, SNUK_TOKEN_VAR))
        init = parse_decl_stmt(parser, false);

    parser_match(parser, SNUK_TOKEN_SEMICOLON);
    cond = parse_expression(parser);
    parser_match(parser, SNUK_TOKEN_SEMICOLON);

    if (parser_match(parser, SNUK_TOKEN_LBRACE)) {
        block = parse_block_stmt(parser);
        return build_for_stmt(parser, init, cond, update, block);
    }

    update = parse_expression(parser);

    parser_expect(parser, SNUK_TOKEN_LBRACE, "expected body of for loop");

    block = parse_block_stmt(parser);

    return build_for_stmt(parser, init, cond, update, block);
}

static SnukStmt *parse_flow_stmt(SnukParser *parser) {
    SnukExpr *value = NULL;
    if (parser->previous.type == SNUK_TOKEN_RETURN)
        value = parse_expression(parser);

    return build_flow_stmt(parser, parser->previous.type, value);
}

static SnukStmt *parse_fn_stmt(SnukParser *parser) {
    SNUK_UNUSED(parser);
    // TODO:
    return NULL;
}

static SnukStmt *parse_type_stmt(SnukParser *parser) {
    SNUK_UNUSED(parser);
    return NULL;
}

static SnukStmt *parse_print_stmt(SnukParser *parser) {
    SnukStmt *print_stmt = build_print_stmt(parser, NULL, parse_expression(parser));
    while (parser_match(parser, SNUK_TOKEN_COMMA))
        print_stmt = build_print_stmt(parser, print_stmt, parse_expression(parser));
    return print_stmt;
}

static SnukStmt *parse_block_stmt(SnukParser *parser) {
    SnukStmt *block_stmt = build_block_stmt(parser, NULL, parse_stmt(parser));
    while (!parser_match(parser, SNUK_TOKEN_RBRACE))
        block_stmt = build_block_stmt(parser, block_stmt, parse_stmt(parser));
    return block_stmt;
}

static SnukStmt *parse_comment_stmt(SnukParser *parser) {
    SnukToken t = parser->previous;
    return build_comment_stmt(parser, t);
}

static SnukExpr *parse_expression(SnukParser *parser) {
    return parse_precedence(parser, PRECEDENCE_ASSIGNMENT);
}

static SnukExpr *parse_precedence(SnukParser *parser, Precedence precedence) {
    parser_advance(parser);
    prefix_fn pfn = get_rule(parser->previous.type)->pfn;
    if (!pfn) {
        parser_error(parser, "expected expression");
        return NULL;
    }

    SnukExpr *left = pfn(parser);
    while (precedence <= get_rule(parser->current.type)->precedence) {
        parser_advance(parser);
        infix_fn ifn = get_rule(parser->previous.type)->ifn;
        left = ifn(parser, left);
    } 

    return left;
}

static SnukExpr *parse_primary(SnukParser *parser) {
    SnukToken t = parser->previous;
    switch (t.type) {
        case SNUK_TOKEN_IDENTIFIER:
            return build_identifier_expr(parser);
        case SNUK_TOKEN_INTEGER:
            return build_int_literal_expr(parser);
        case SNUK_TOKEN_FLOAT:
            return build_float_literal_expr(parser);
        case SNUK_TOKEN_STRING:
            return build_string_literal_expr(parser);
        case SNUK_TOKEN_TRUE:
        case SNUK_TOKEN_FALSE:
            return build_bool_expr(parser);
        case SNUK_TOKEN_NULL:
            return build_null_expr(parser);
        case SNUK_TOKEN_NAN:
            // TODO:
        case SNUK_TOKEN_INF:
            // TODO:
            break;
        default:
            // TODO:
            parser_error(parser, "unexpected expression");
            break;
    }

    return NULL;
}

static SnukExpr *parse_grouping(SnukParser *parser) {
    SnukExpr *expr = parse_expression(parser);
    parser_expect(parser, SNUK_TOKEN_RPAREN, "expected ')'");
    return expr;
}

static SnukExpr *parse_unary(SnukParser *parser) {
    SnukToken op = parser->previous;
    SnukExpr *right = parse_precedence(parser, PRECEDENCE_UNARY);
    return build_unary_expr(parser, op.type, right);
}

static SnukExpr *parse_binary(SnukParser *parser, SnukExpr *left) {
    SnukToken op = parser->previous;
    ParseRule *rule = get_rule(op.type);
    SnukExpr *right = parse_precedence(parser, rule->precedence + 1);
    return build_binary_expr(parser, op.type, left, right);
}

static SnukExpr *parse_assignment(SnukParser *parser, SnukExpr *left) {
    if (left->type != SNUK_EXPR_IDENTIFIER) {
        parser_error(parser, "invalid assignment target");
        return NULL;
    }
    SnukExpr *value = parse_precedence(parser, PRECEDENCE_ASSIGNMENT);
    return build_assign_expr(parser, left, value);
}

static void parser_error(SnukParser *parser, const char *err_msg) {
    if (parser->panic_mode) return;

    parser->panic_mode = true;
    parser->had_error = true;

    SnukToken t = parser->current;
    snuk_eprint("%lu:%lu error", t.line, t.col);
    if (t.type == SNUK_TOKEN_EOF) snuk_eprint(" at end");
    else snuk_eprint(" at '%.*s'", t.string_literal.length, t.string_literal.value);
    snuk_eprintln(" at '%s", err_msg);
}

static void parser_sync(SnukParser *parser) {
    // TODO: skipping errors
    parser->panic_mode = false;
    return;

    while (parser->current.type != SNUK_TOKEN_EOF) {
        switch (parser->previous.type) {
            case SNUK_TOKEN_SEMICOLON:
            case SNUK_TOKEN_SLCOMMENT:
            case SNUK_TOKEN_MLCOMMENT:
                return;
            default:
                break;
        }

        switch (parser->current.type) {
            case SNUK_TOKEN_IF:
            case SNUK_TOKEN_WHILE:
            case SNUK_TOKEN_FOR:
            case SNUK_TOKEN_DO:
            case SNUK_TOKEN_RETURN:
            case SNUK_TOKEN_VAR:
            case SNUK_TOKEN_CONST:
            case SNUK_TOKEN_LBRACE:
                return;
            default:
                break;
        }

        parser_advance(parser);
    }
}

void snuk_parser_log_stmt(SnukStmt *stmt) {
    if (!stmt) return;
    log_trace("Statement type: %s", snuk_parser_stmt_type_to_string(stmt->type));

    uint64_t count;
    switch (stmt->type) {
        case SNUK_STMT_EXPR:
            log_trace("Expr:", NULL);
            snuk_parser_log_expr(stmt->expr_stmt);
            break;
        case SNUK_STMT_VAR_DECL:
            log_trace("var %.*s = ", stmt->decl_stmt.length, stmt->decl_stmt.name);
            snuk_parser_log_expr(stmt->decl_stmt.init);
            break;
        case SNUK_STMT_CONST_DECL:
            log_trace("const %.*s = ", stmt->decl_stmt.length, stmt->decl_stmt.name);
            snuk_parser_log_expr(stmt->decl_stmt.init);
            break;
        case SNUK_STMT_IF:
            log_trace("if:", NULL);
            snuk_parser_log_expr(stmt->if_stmt.condition);
            snuk_parser_log_stmt(stmt->if_stmt.then_branch);
            snuk_parser_log_stmt(stmt->if_stmt.else_branch);
            break;
        case SNUK_STMT_MATCH:
            log_trace("match:", NULL);
            break;
        case SNUK_STMT_WHILE:
            log_trace("while:", NULL);
            snuk_parser_log_expr(stmt->while_stmt.condition);
            snuk_parser_log_stmt(stmt->while_stmt.block);
            break;
        case SNUK_STMT_DO_WHILE:
            log_trace("do:", NULL);
            snuk_parser_log_stmt(stmt->while_stmt.block);
            snuk_parser_log_expr(stmt->while_stmt.condition);
            break;
        case SNUK_STMT_FOR:
            log_trace("for:", NULL);
            snuk_parser_log_stmt(stmt->for_stmt.init);
            snuk_parser_log_expr(stmt->for_stmt.cond);
            snuk_parser_log_expr(stmt->for_stmt.update);
            snuk_parser_log_stmt(stmt->for_stmt.block);
            break;
        case SNUK_STMT_RETURN:
            log_trace("return:", NULL);
            snuk_parser_log_expr(stmt->return_stmt);
            break;
        case SNUK_STMT_BREAK:
            log_trace("break", NULL);
            break;
        case SNUK_STMT_CONTINUE:
            log_trace("continue", NULL);
            break;
        case SNUK_STMT_FN:
            log_trace("function:", NULL);
            break;
        case SNUK_STMT_TYPE:
            log_trace("type:", NULL);
            break;
        case SNUK_STMT_PRINT:
            log_trace("print:", NULL);
            count = snuk_darray_get_length(stmt->print_stmt.exprs);
            for (uint64_t i = 0; i < count; ++i)
                snuk_parser_log_expr(stmt->print_stmt.exprs[i]);
            break;
        case SNUK_STMT_BLOCK:
            log_trace("block:", NULL);
            count = snuk_darray_get_length(stmt->block_stmt.stmts);
            for (uint64_t i = 0; i < count; ++i)
                snuk_parser_log_stmt(stmt->block_stmt.stmts[i]);
            break;
        case SNUK_STMT_SLCOMMENT:
            log_trace("single line comment: %.*s", stmt->comment_stmt.length, stmt->comment_stmt.comment);
            break;
        case SNUK_STMT_MLCOMMENT:
            log_trace("multi-line comment: %.*s", stmt->comment_stmt.length, stmt->comment_stmt.comment);
            break;
        default:
            break;
    }
}

void snuk_parser_log_expr(SnukExpr *expr) {
    if (!expr) return;
    log_trace("Expression type: %s", snuk_parser_expr_type_to_string(expr->type));

    switch (expr->type) {
        case SNUK_EXPR_IDENTIFIER:
            log_trace("Identifier: %.*s", expr->identifier.length, expr->identifier.name);
            break;
        case SNUK_EXPR_INT_LITERAL:
            log_trace("Integer: %ld", expr->int_literal);
            break;
        case SNUK_EXPR_FLOAT_LITERAL:
            log_trace("Float: %lf", expr->float_literal);
            break;
        case SNUK_EXPR_STRING_LITERAL:
            log_trace("String: %.*s", expr->string_literal.length, expr->string_literal.value);
            break;
        case SNUK_EXPR_TRUE_LITERAL:
        case SNUK_EXPR_FALSE_LITERAL:
            log_trace("Bool: %s", expr->type == SNUK_EXPR_TRUE_LITERAL? "true" : "false");
            break;
        case SNUK_EXPR_NULL_LITERAL:
            log_trace("Null", NULL);
            break;
        case SNUK_EXPR_UNARY:
            log_trace("Unary:", NULL);
            log_trace("%s", snuk_lexer_token_type_to_string(expr->unary.op));
            snuk_parser_log_expr(expr->unary.operand);
            break;
        case SNUK_EXPR_BINARY:
            log_trace("Binary:", NULL);
            snuk_parser_log_expr(expr->binary.left);
            log_trace("%s", snuk_lexer_token_type_to_string(expr->binary.op));
            snuk_parser_log_expr(expr->binary.right);
            break;
        case SNUK_EXPR_CALL:
            log_trace("call: %.*s", expr->identifier.length, expr->identifier.name);
            break;
        case SNUK_EXPR_MEMBER:
            log_trace("Member:", NULL);
            break;
        case SNUK_EXPR_INDEX:
            log_trace("Index:", NULL);
            break;
        case SNUK_EXPR_ASSIGN:
            snuk_parser_log_expr(expr->assign.identifier);
            snuk_parser_log_expr(expr->assign.value);
        default:
            break;
    }
}

const char *snuk_parser_stmt_type_to_string(SnukStmtType type) {
    switch (type) {
        case SNUK_STMT_EXPR:
            return SNUK_STRINGIFY(SNUK_STMT_EXPR);
        case SNUK_STMT_VAR_DECL:
            return SNUK_STRINGIFY(SNUK_STMT_VAR_DECL);
        case SNUK_STMT_CONST_DECL:
            return SNUK_STRINGIFY(SNUK_STMT_CONST_DECL);
        case SNUK_STMT_IF:
            return SNUK_STRINGIFY(SNUK_STMT_IF);
        case SNUK_STMT_MATCH:
            return SNUK_STRINGIFY(SNUK_STMT_MATCH);
        case SNUK_STMT_WHILE:
            return SNUK_STRINGIFY(SNUK_STMT_WHILE);
        case SNUK_STMT_DO_WHILE:
            return SNUK_STRINGIFY(SNUK_STMT_DO_WHILE);
        case SNUK_STMT_FOR:
            return SNUK_STRINGIFY(SNUK_STMT_FOR);
        case SNUK_STMT_RETURN:
            return SNUK_STRINGIFY(SNUK_STMT_RETURN);
        case SNUK_STMT_BREAK:
            return SNUK_STRINGIFY(SNUK_STMT_BREAK);
        case SNUK_STMT_CONTINUE:
            return SNUK_STRINGIFY(SNUK_STMT_CONTINUE);
        case SNUK_STMT_FN:
            return SNUK_STRINGIFY(SNUK_STMT_FN);
        case SNUK_STMT_TYPE:
            return SNUK_STRINGIFY(SNUK_STMT_TYPE);
        case SNUK_STMT_PRINT:
            return SNUK_STRINGIFY(SNUK_STMT_PRINT);
        case SNUK_STMT_BLOCK:
            return SNUK_STRINGIFY(SNUK_STMT_BLOCK);
        case SNUK_STMT_SLCOMMENT:
            return SNUK_STRINGIFY(SNUK_STMT_SLCOMMENT);
        case SNUK_STMT_MLCOMMENT:
            return SNUK_STRINGIFY(SNUK_STMT_MLCOMMENT);
        case SNUK_STMT_MAX:
            return SNUK_STRINGIFY(SNUK_STMT_MAX);
        default:
            return "Unkown statement type";
    }
}

const char *snuk_parser_expr_type_to_string(SnukExprType type) {
    switch (type) {
        case SNUK_EXPR_IDENTIFIER:
            return SNUK_STRINGIFY(SNUK_EXPR_IDENTIFIER);
        case SNUK_EXPR_INT_LITERAL:
            return SNUK_STRINGIFY(SNUK_EXPR_INT_LITERAL);
        case SNUK_EXPR_FLOAT_LITERAL:
            return SNUK_STRINGIFY(SNUK_EXPR_FLOAT_LITERAL);
        case SNUK_EXPR_STRING_LITERAL:
            return SNUK_STRINGIFY(SNUK_EXPR_STRING_LITERAL);
        case SNUK_EXPR_TRUE_LITERAL:
            return SNUK_STRINGIFY(SNUK_EXPR_TRUE_LITERAL);
        case SNUK_EXPR_FALSE_LITERAL:
            return SNUK_STRINGIFY(SNUK_EXPR_FALSE_LITERAL);
        case SNUK_EXPR_NULL_LITERAL:
            return SNUK_STRINGIFY(SNUK_EXPR_NULL_LITERAL);
        case SNUK_EXPR_UNARY:
            return SNUK_STRINGIFY(SNUK_EXPR_UNARY);
        case SNUK_EXPR_BINARY:
            return SNUK_STRINGIFY(SNUK_EXPR_BINARY);
        case SNUK_EXPR_ASSIGN:
            return SNUK_STRINGIFY(SNUK_EXPR_ASSIGN);
        case SNUK_EXPR_CALL:
            return SNUK_STRINGIFY(SNUK_EXPR_CALL);
        case SNUK_EXPR_MEMBER:
            return SNUK_STRINGIFY(SNUK_EXPR_MEMBER);
        case SNUK_EXPR_INDEX:
            return SNUK_STRINGIFY(SNUK_EXPR_INDEX);
        case SNUK_EXPR_MAX:
            return SNUK_STRINGIFY(SNUK_EXPR_MAX);
        default:
            return "Unknown expression type";
    }
}
