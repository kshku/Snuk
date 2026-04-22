#include "parser.h"

#include "parser_helper.h"

#include "io.h"
#include "memory.h"
#include "logger.h"
#include "darray.h"

/**
 * @brief Parse the next top-level item.
 */
SnukItem *snuk_parser_next_item(SnukParser *parser) {
    if (parser->current.type == SNUK_TOKEN_EOF) return NULL;
    SnukItem *item = parse_item(parser);
    if (parser->panic_mode) parser_sync(parser);
    return item;
}

/**
 * @brief Dispatch item parsing based on the current token.
 */
static SnukItem *parse_item(SnukParser *parser) {
    if (parser_match(parser, SNUK_TOKEN_VAR) || parser_match(parser, SNUK_TOKEN_CONST))
        return parse_decl_item(parser, parser->previous.type == SNUK_TOKEN_CONST);

    if (parser_match(parser, SNUK_TOKEN_RETURN) || parser_match(parser, SNUK_TOKEN_CONTINUE)
            || parser_match(parser, SNUK_TOKEN_BREAK))
        return parse_flow_item(parser);

    if (parser_match(parser, SNUK_TOKEN_FN)) return parse_fn_item(parser);

    if (parser_match(parser, SNUK_TOKEN_TYPE)) return parse_type_item(parser);

    if (parser_match(parser, SNUK_TOKEN_PRINT)) return parse_print_item(parser);

    if (parser_match(parser, SNUK_TOKEN_BLOCK_COMMENT) || parser_match(parser, SNUK_TOKEN_LINE_COMMENT))
        return parse_comment_item(parser);

    return parse_expr_item(parser);
}

/**
 * @brief Parse an expression item.
 */
static SnukItem *parse_expr_item(SnukParser *parser) {
    return build_expr_item(parser, parse_expression(parser));
}

/**
 * @brief Parse a variable or constant declaration item.
 */
static SnukItem *parse_decl_item(SnukParser *parser, bool is_const) {
    parser_expect(parser, SNUK_TOKEN_IDENTIFIER, "expected an identifier");
    SnukExpr *identifier = parse_primary(parser);

    SnukType *type = NULL;
    if (parser_match(parser, SNUK_TOKEN_COLON))
        type = parse_type_annot(parser);
    else
        type = build_any_type(parser);

    SnukExpr *init = NULL;
    if (parser_match(parser, SNUK_TOKEN_ASSIGN)) init = parse_expression(parser);
    else init = build_null_expr(parser);

    return build_decl_item(parser, identifier, type, init, is_const);
}

/**
 * @brief Parse return, break, or continue items.
 */
static SnukItem *parse_flow_item(SnukParser *parser) {
    SnukTokenType type = parser->previous.type;
    SnukExpr *value = NULL;
    if (parser->previous.type == SNUK_TOKEN_RETURN || parser->previous.type == SNUK_TOKEN_BREAK)
        // TODO: look for delimiter, value is optional
        // TODO: break and return items should be at the end of block only?
        value = parse_expression(parser);

    return build_flow_item(parser, type, value);
}

/**
 * @brief Parse a function declaration item.
 */
static SnukItem *parse_fn_item(SnukParser *parser) {
    parser_expect(parser, SNUK_TOKEN_IDENTIFIER, "expected function name");
    SnukExpr *identifier = parse_primary(parser);

    SnukParam **params = snuk_darray_create(SnukParam *);
    parser_expect(parser, SNUK_TOKEN_LPAREN, "expected '('");
    while (!parser_match(parser, SNUK_TOKEN_RPAREN)) {
        parser_expect(parser, SNUK_TOKEN_IDENTIFIER, "expected parameter name");

        SnukExpr *name = parse_primary(parser);
        SnukExpr *default_value = NULL;
        SnukType *type = NULL;

        if (parser_match(parser, SNUK_TOKEN_COLON))
            type = parse_type_annot(parser);
        else
            type = build_any_type(parser);

        if (parser_match(parser, SNUK_TOKEN_ASSIGN))
            default_value = parse_expression(parser);

        snuk_darray_push(&params, build_param(parser,name, type, default_value));

        if (!parser_check(parser, SNUK_TOKEN_RPAREN))
            parser_expect(parser, SNUK_TOKEN_COMMA, "expected comma");
    }

    SnukType *ret_type = NULL;
    if (parser_match(parser, SNUK_TOKEN_ARROW))
        ret_type = parse_type_annot(parser);

    parser_expect(parser, SNUK_TOKEN_LBRACE, "expected body of function");
    SnukExpr *body = parse_block(parser);

    return build_fn_item(parser, identifier, params, body, ret_type);
}

/**
 * @brief Parse a type declaration item.
 */
static SnukItem *parse_type_item(SnukParser *parser) {
    // TODO:
    parser_expect(parser, SNUK_TOKEN_IDENTIFIER, "expected name of type");

    SnukExpr *identifier = parse_primary(parser);
    SnukItem **vars = snuk_darray_create(SnukItem *);
    SnukItem **fns = snuk_darray_create(SnukItem *);

    parser_expect(parser, SNUK_TOKEN_LBRACE, "expected '{'");

    while (!parser_match(parser, SNUK_TOKEN_RBRACE)
            && parser->current.type !=  SNUK_TOKEN_EOF) {

        if (parser_match(parser, SNUK_TOKEN_VAR) || parser_match(parser, SNUK_TOKEN_CONST)) {
            snuk_darray_push(&vars,
                    parse_decl_item(parser, parser->previous.type == SNUK_TOKEN_CONST));
        } else if (parser_match(parser, SNUK_TOKEN_FN)) {
            snuk_darray_push(&fns, parse_fn_item(parser));
        } else {
            parser_error(parser, "Unexpected token");
        }
    }

    if (parser->previous.type != SNUK_TOKEN_RBRACE) {
        parser_error(parser, "expected '}'");
        return NULL;
    }

    return build_type_item(parser, identifier, vars, fns);
}

/**
 * @brief Parse a print item.
 */
static SnukItem *parse_print_item(SnukParser *parser) {
    SnukItem *print_item = build_print_item(parser, NULL, parse_expression(parser));
    while (parser_match(parser, SNUK_TOKEN_COMMA))
        print_item = build_print_item(parser, print_item, parse_expression(parser));
    return print_item;
}

/**
 * @brief Parse a source comment as a item.
 */
static SnukItem *parse_comment_item(SnukParser *parser) {
    SnukToken t = parser->previous;
    return build_comment_item(parser, t);
}

/**
 * @breif Parse a type annotation.
 */
static SnukType *parse_type_annot(SnukParser *parser) {
    if (parser_match(parser, SNUK_TOKEN_FN)) {
        SnukType *type = build_fn_type(parser, NULL, NULL, NULL);
        parser_expect(parser, SNUK_TOKEN_LPAREN, "exptected '('");
        while (!parser_match(parser, SNUK_TOKEN_RPAREN)) {
            SnukType *param = parse_type_annot(parser);
            type = build_fn_type(parser, type, param, NULL);
            if (!parser_check(parser, SNUK_TOKEN_RPAREN))
                parser_expect(parser, SNUK_TOKEN_COMMA, "expected ','");
        }

        SnukType *ret_type = NULL;
        if (parser_match(parser, SNUK_TOKEN_ARROW))
            ret_type = parse_type_annot(parser);

        type = build_fn_type(parser, type, NULL, ret_type);
        return type;
    }

    if (parser_match(parser, SNUK_TOKEN_ANY))
        return build_any_type(parser);

    parser_expect(parser, SNUK_TOKEN_IDENTIFIER, "unexpected type");

    return build_named_type(parser, parser->previous.string_literal);
}

/**
 * @brief Parse an expression from the lowest precedence.
 */
static SnukExpr *parse_expression(SnukParser *parser) {
    return parse_precedence(parser, PRECEDENCE_ASSIGNMENT);
}

/**
 * @brief Parse an expression at or above the given precedence.
 */
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

/**
 * @brief Parse a primary expression.
 */
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

/**
 * @brief Parse a grouped expression.
 */
static SnukExpr *parse_grouping(SnukParser *parser) {
    SnukExpr *expr = parse_expression(parser);
    parser_expect(parser, SNUK_TOKEN_RPAREN, "expected ')'");
    return expr;
}

/**
 * @brief Parse a unary expression.
 */
static SnukExpr *parse_unary(SnukParser *parser) {
    SnukToken op = parser->previous;
    SnukExpr *right = parse_precedence(parser, PRECEDENCE_UNARY);
    return build_unary_expr(parser, op.type, right);
}

/**
 * @brief Parse a binary expression.
 */
static SnukExpr *parse_binary(SnukParser *parser, SnukExpr *left) {
    SnukToken op = parser->previous;
    ParseRule *rule = get_rule(op.type);
    SnukExpr *right = parse_precedence(parser, rule->precedence + 1);
    return build_binary_expr(parser, op.type, left, right);
}

/**
 * @brief Parse an assignment expression.
 */
static SnukExpr *parse_assignment(SnukParser *parser, SnukExpr *left) {
    if (left->type != SNUK_EXPR_IDENTIFIER) {
        parser_error(parser, "invalid assignment target");
        return NULL;
    }
    SnukExpr *value = parse_precedence(parser, PRECEDENCE_ASSIGNMENT);
    return build_assign_expr(parser, left, value);
}

/**
 * @brief Parse an compound assignment expression.
 */
static SnukExpr *parse_compound_assignment(SnukParser *parser, SnukExpr *left) {
    if (left->type != SNUK_EXPR_IDENTIFIER) {
        parser_error(parser, "invalid assignment target");
        return NULL;
    }
    SnukTokenType op = parser->previous.type;
    SnukExpr *value = parse_precedence(parser, PRECEDENCE_ASSIGNMENT);
    return build_compound_assign_expr(parser, op, left, value);
}

/**
 * @brief Parse an if expression.
 */
static SnukExpr *parse_if(SnukParser *parser) {
    SnukExpr *condition = parse_expression(parser);
    parser_expect(parser, SNUK_TOKEN_LBRACE, "expected '{'");
    SnukExpr *then_block = parse_block(parser);
    SnukExpr *else_block = NULL;
    if (parser_match(parser, SNUK_TOKEN_ELSE)) {
        if (parser_match(parser, SNUK_TOKEN_IF))
            else_block = parse_if(parser);
        parser_expect(parser, SNUK_TOKEN_LBRACE, "expected '{'");
        else_block = parse_block(parser);
    }
    return build_if_expr(parser, condition, then_block, else_block);
}

/**
 * @brief Parse an match expression.
 */
static SnukExpr *parse_match(SnukParser *parser) {
    // TODO:
    return build_match_expr(parser, NULL);
}

/**
 * @brief Parse an while or do while loop expression.
 */
static SnukExpr *parse_while(SnukParser *parser) {
    SnukExpr *condition = NULL;
    SnukExpr *body = NULL;

    if (parser->previous.type == SNUK_TOKEN_DO) {
        parser_expect(parser, SNUK_TOKEN_LBRACE, "expected '{'");
        body = parse_block(parser);
        parser_expect(parser, SNUK_TOKEN_WHILE, "expected while");
        condition = parse_expression(parser);
        return build_while_expr(parser, condition, body, true);
    }

    condition = parse_expression(parser);
    parser_expect(parser, SNUK_TOKEN_LBRACE, "expected '{'");
    body = parse_block(parser);
    return build_while_expr(parser, condition, body, false);
}

/**
 * @brief Parse an for loop expression.
 */
static SnukExpr *parse_for(SnukParser *parser) {
    SnukItem *init = NULL;
    SnukExpr *condition = NULL;
    SnukExpr *update = NULL;
    SnukExpr *body = NULL;

    if (parser_match(parser, SNUK_TOKEN_LBRACE)) {
        // infinity loop
        body = parse_block(parser);
        return build_for_expr(parser, init, condition, update, body);
    }

    // TODO: allow const?
    if (parser_match(parser, SNUK_TOKEN_VAR))
        parse_decl_item(parser, false);

    parser_match(parser, SNUK_TOKEN_SEMICOLON);
    condition = parse_expression(parser);
    parser_match(parser, SNUK_TOKEN_SEMICOLON);

    if (parser_match(parser, SNUK_TOKEN_LBRACE)) {
        body = parse_block(parser);
        return build_for_expr(parser, init, condition, update, body);
    }

    update = parse_expression(parser);

    if (parser_match(parser, SNUK_TOKEN_SEMICOLON) && !init) {
        init = build_expr_item(parser, condition);
        condition = update;
        update = parse_expression(parser);
    }

    parser_expect(parser, SNUK_TOKEN_LBRACE, "expected body of for loop");
    body = parse_block(parser);

    return build_for_expr(parser, init, condition, update, body);
}

/**
 * @brief Parse an function expression.
 */
static SnukExpr *parse_fn(SnukParser *parser) {
    SnukParam **params = snuk_darray_create(SnukParam *);
    parser_expect(parser, SNUK_TOKEN_LPAREN, "expected '('");
    while (!parser_match(parser, SNUK_TOKEN_RPAREN)
            && parser->current.type != SNUK_TOKEN_EOF) {
        parser_expect(parser, SNUK_TOKEN_IDENTIFIER, "expected parameter name");

        SnukExpr *name = parse_primary(parser);
        SnukExpr *default_value = NULL;
        SnukType *type = NULL;

        if (parser_match(parser, SNUK_TOKEN_COLON))
            type = parse_type_annot(parser);
        else
            type = build_any_type(parser);

        if (parser_match(parser, SNUK_TOKEN_ASSIGN))
            default_value = parse_expression(parser);

        snuk_darray_push(&params, build_param(parser,name, type, default_value));

        if (!parser_check(parser, SNUK_TOKEN_RPAREN))
            parser_expect(parser, SNUK_TOKEN_COMMA, "expected comma");
    }

    if (parser->previous.type != SNUK_TOKEN_RPAREN) {
        parser_error(parser, "expected ')'");
        return NULL;
    }

    SnukType *ret_type = NULL;
    if (parser_match(parser, SNUK_TOKEN_ARROW))
        ret_type = parse_type_annot(parser);

    parser_expect(parser, SNUK_TOKEN_LBRACE, "expected body of function");
    SnukExpr *body = parse_block(parser);

    return build_fn_expr(parser, params, body, ret_type);
}

/**
 * @brief Parse an type expression.
 */
static SnukExpr *parse_type(SnukParser *parser) {
    // TODO:
    return build_type_expr(parser, NULL, NULL);
}

/**
 * @brief Parse an block expression.
 */
static SnukExpr *parse_block(SnukParser *parser) {
    SnukExpr *block_expr = build_block_expr(parser, NULL, NULL);

    while (!parser_match(parser, SNUK_TOKEN_RBRACE)
            && parser->current.type != SNUK_TOKEN_EOF)
        block_expr = build_block_expr(parser, block_expr, parse_item(parser));

    if (parser->previous.type != SNUK_TOKEN_RBRACE) {
        parser_error(parser, "block was not closed");
        return NULL;
    }

    return block_expr;
}

/**
 * @brief Report a parser error and enter panic mode.
 */
static void parser_error(SnukParser *parser, const char *err_msg) {
    if (parser->panic_mode) return;

    parser->panic_mode = true;
    parser->had_error = true;

    SnukToken t = parser->current;
    snuk_eprint("%lu:%lu error", t.line, t.col);
    if (t.type == SNUK_TOKEN_EOF) snuk_eprint(" at end");
    else snuk_eprint(" at '"SNUK_STRING_VIEW_FORMAT"'", SNUK_STRING_VIEW_ARG(t.string_literal));
    snuk_eprintln(" at '%s", err_msg);
}

/**
 * @brief Recover parser state after an error.
 */
static void parser_sync(SnukParser *parser) {
    // TODO: skipping errors
    parser->panic_mode = false;
    return;

    while (parser->current.type != SNUK_TOKEN_EOF) {
        if (parser->previous.type == SNUK_TOKEN_SEMICOLON)
            return;

        switch (parser->current.type) {
            case SNUK_TOKEN_IF:
            case SNUK_TOKEN_WHILE:
            case SNUK_TOKEN_FOR:
            case SNUK_TOKEN_DO:
            case SNUK_TOKEN_RETURN:
            case SNUK_TOKEN_VAR:
            case SNUK_TOKEN_CONST:
            case SNUK_TOKEN_LBRACE:
            case SNUK_TOKEN_LINE_COMMENT:
            case SNUK_TOKEN_BLOCK_COMMENT:
                return;
            default:
                break;
        }

        parser_advance(parser);
    }
}

/**
 * @brief Log a item tree.
 */
void snuk_parser_log_item(SnukItem *item) {
    if (!item) return;
    log_trace("item type: %s", snuk_parser_item_type_to_string(item->type));

    uint64_t count;
    switch (item->type) {
        case SNUK_ITEM_EXPR:
            log_trace("Expr:", NULL);
            snuk_parser_log_expr(item->expr);
            break;
        case SNUK_ITEM_VAR_DECL:
            log_trace("var: ", NULL);
            snuk_parser_log_expr(item->var_decl.identifier);
            if (item->var_decl.type) log_trace("type: ", NULL);
            snuk_parser_log_type(item->var_decl.type);
            snuk_parser_log_expr(item->var_decl.init);
            break;
        case SNUK_ITEM_CONST_DECL:
            log_trace("const: ", NULL);
            snuk_parser_log_expr(item->var_decl.identifier);
            if (item->var_decl.type) log_trace("type: ", NULL);
            snuk_parser_log_type(item->var_decl.type);
            snuk_parser_log_expr(item->var_decl.init);
            break;
        case SNUK_ITEM_RETURN:
            log_trace("return:", NULL);
            snuk_parser_log_expr(item->expr);
            break;
        case SNUK_ITEM_BREAK:
            log_trace("break", NULL);
            snuk_parser_log_expr(item->expr);
            break;
        case SNUK_ITEM_CONTINUE:
            log_trace("continue", NULL);
            break;
        case SNUK_ITEM_FN_DECL:
            log_trace("function:", NULL);
            snuk_parser_log_expr(item->fn_decl.identifier);
            count = snuk_darray_get_length(item->fn_decl.params);
            for (uint64_t i = 0; i < count; ++i)
                snuk_parser_log_param(item->fn_decl.params[i]);
            snuk_parser_log_expr(item->fn_decl.body);
            break;
        case SNUK_ITEM_TYPE_DECL:
            log_trace("type:", NULL);
            snuk_parser_log_expr(item->type_decl.identifier);
            count = snuk_darray_get_length(item->type_decl.vars);
            for (uint64_t i = 0; i < count; ++i)
                snuk_parser_log_item(item->type_decl.vars[i]);
            count = snuk_darray_get_length(item->type_decl.fns);
            for (uint64_t i = 0; i < count; ++i)
                snuk_parser_log_item(item->type_decl.fns[i]);
            break;
        case SNUK_ITEM_PRINT:
            log_trace("print:", NULL);
            count = snuk_darray_get_length(item->print_exprs);
            for (uint64_t i = 0; i < count; ++i)
                snuk_parser_log_expr(item->print_exprs[i]);
            break;
        case SNUK_ITEM_LINE_COMMENT:
            log_trace("single line comment: "SNUK_STRING_VIEW_FORMAT, SNUK_STRING_VIEW_ARG(item->comment));
            break;
        case SNUK_ITEM_BLOCK_COMMENT:
            log_trace("multi-line comment: "SNUK_STRING_VIEW_FORMAT, SNUK_STRING_VIEW_ARG(item->comment));
            break;
        default:
            break;
    }
}

/**
 * @brief Log an expression tree.
 */
void snuk_parser_log_expr(SnukExpr *expr) {
    if (!expr) return;
    log_trace("Expression type: %s", snuk_parser_expr_type_to_string(expr->type));

    uint64_t count;
    switch (expr->type) {
        case SNUK_EXPR_IDENTIFIER:
            log_trace("Identifier: "SNUK_STRING_VIEW_FORMAT, SNUK_STRING_VIEW_ARG(expr->identifier));
            break;
        case SNUK_EXPR_INT:
            log_trace("Integer: %ld", expr->int_literal);
            break;
        case SNUK_EXPR_FLOAT:
            log_trace("Float: %lf", expr->float_literal);
            break;
        case SNUK_EXPR_STRING:
            log_trace("String: "SNUK_STRING_VIEW_FORMAT, SNUK_STRING_VIEW_ARG(expr->string_literal));
            break;
        case SNUK_EXPR_BOOL:
            log_trace("Bool: %s", expr->bool_literal ? "true" : "false");
            break;
        case SNUK_EXPR_NULL:
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
        case SNUK_EXPR_ASSIGN:
            snuk_parser_log_expr(expr->assign.identifier);
            snuk_parser_log_expr(expr->assign.value);
            break;
        case SNUK_EXPR_COMPOUND_ASSIGN:
            snuk_parser_log_expr(expr->compound_assign.identifier);
            log_trace("%s", snuk_lexer_token_type_to_string(expr->compound_assign.op));
            snuk_parser_log_expr(expr->compound_assign.value);
            break;
        case SNUK_EXPR_IF:
            log_trace("if:", NULL);
            snuk_parser_log_expr(expr->if_else.condition);
            log_trace("then:", NULL);
            snuk_parser_log_expr(expr->if_else.then_block);
            log_trace("else:", NULL);
            snuk_parser_log_expr(expr->if_else.else_block);
            break;
        case SNUK_EXPR_MATCH:
            // TODO:
            log_trace("match expression:", NULL);
            break;
        case SNUK_EXPR_WHILE:
            log_trace("while:", NULL);
            snuk_parser_log_expr(expr->while_loop.condition);
            log_trace("run:", NULL);
            snuk_parser_log_expr(expr->while_loop.body);
            break;
        case SNUK_EXPR_DO_WHILE:
            log_trace("do:", NULL);
            snuk_parser_log_expr(expr->while_loop.body);
            log_trace("while:", NULL);
            snuk_parser_log_expr(expr->while_loop.condition);
            break;
        case SNUK_EXPR_FOR:
            log_trace("for:", NULL);
            snuk_parser_log_item(expr->for_loop.init);
            snuk_parser_log_expr(expr->for_loop.condition);
            snuk_parser_log_expr(expr->for_loop.update);
            log_trace("run:", NULL);
            snuk_parser_log_expr(expr->for_loop.body);
            break;
        case SNUK_EXPR_FN:
            // TODO:
            log_trace("fn expression:", NULL);
            break;
        case SNUK_EXPR_TYPE:
            // TODO:
            log_trace("type expression:", NULL);
            break;
        case SNUK_EXPR_BLOCK:
            log_trace("block expression:", NULL);
            count = snuk_darray_get_length(expr->block_items);
            for (uint64_t i = 0; i < count; ++i)
                snuk_parser_log_item(expr->block_items[i]);
            break;
        case SNUK_EXPR_CALL:
            // TODO:
            log_trace("call: "SNUK_STRING_VIEW_FORMAT, SNUK_STRING_VIEW_ARG(expr->identifier));
            break;
        case SNUK_EXPR_MEMBER:
            // TODO:
            log_trace("Member:", NULL);
            break;
        case SNUK_EXPR_INDEX:
            // TODO:
            log_trace("Index:", NULL);
            break;
        default:
            break;
    }
}

/**
 * @brief Log a function parameter.
 */
void snuk_parser_log_param(SnukParam *param) {
    if (!param) return;
    log_trace("param: ", NULL);
    snuk_parser_log_expr(param->identifier);
    if (param->type) log_trace("type: ", NULL);
    snuk_parser_log_type(param->type);
    snuk_parser_log_expr(param->default_value);
}

/**
 * @brief Log a type annotation.
 */
void snuk_parser_log_type(SnukType *type) {
    if (!type) {
        log_trace("void type", NULL);
        return;
    }

    uint64_t count;
    switch (type->type) {
        case TYPE_ANY:
            log_trace("type type: %s", SNUK_STRINGIFY(TYPE_ANY));
            break;
        case TYPE_NAMED:
            log_trace("type type: %s", SNUK_STRINGIFY(TYPE_NAMED));
            log_trace("type name: "SNUK_STRING_VIEW_FORMAT, SNUK_STRING_VIEW_ARG(type->name));
            break;
        case TYPE_FN:
            log_trace("type type: %s", SNUK_STRINGIFY(TYPE_FN));
            log_trace("param types:", NULL);
            count = snuk_darray_get_length(type->fn.param_types);
            for (uint64_t i = 0; i < count; ++i)
                snuk_parser_log_type(type->fn.param_types[i]);
            log_trace("return type:", NULL);
            snuk_parser_log_type(type->fn.return_type);
            break;
        default:
            break;
    }
}

/**
 * @brief Convert a item type to a string.
 */
const char *snuk_parser_item_type_to_string(SnukItemType type) {
    switch (type) {
        case SNUK_ITEM_EXPR:
            return SNUK_STRINGIFY(SNUK_ITEM_EXPR);
        case SNUK_ITEM_VAR_DECL:
            return SNUK_STRINGIFY(SNUK_ITEM_VAR_DECL);
        case SNUK_ITEM_CONST_DECL:
            return SNUK_STRINGIFY(SNUK_ITEM_CONST_DECL);
        case SNUK_ITEM_FN_DECL:
            return SNUK_STRINGIFY(SNUK_ITEM_FN_DECL);
        case SNUK_ITEM_TYPE_DECL:
            return SNUK_STRINGIFY(SNUK_ITEM_TYPE_DECL);
        case SNUK_ITEM_PRINT:
            return SNUK_STRINGIFY(SNUK_ITEM_PRINT);
        case SNUK_ITEM_RETURN:
            return SNUK_STRINGIFY(SNUK_ITEM_RETURN);
        case SNUK_ITEM_BREAK:
            return SNUK_STRINGIFY(SNUK_ITEM_BREAK);
        case SNUK_ITEM_CONTINUE:
            return SNUK_STRINGIFY(SNUK_ITEM_CONTINUE);
        case SNUK_ITEM_LINE_COMMENT:
            return SNUK_STRINGIFY(SNUK_ITEM_LINE_COMMENT);
        case SNUK_ITEM_BLOCK_COMMENT:
            return SNUK_STRINGIFY(SNUK_ITEM_BLOCK_COMMENT);
        case SNUK_ITEM_MAX:
            return SNUK_STRINGIFY(SNUK_ITEM_MAX);
        default:
            return "Unkown item type";
    }
}

/**
 * @brief Convert an expression type to a string.
 */
const char *snuk_parser_expr_type_to_string(SnukExprType type) {
    switch (type) {
        case SNUK_EXPR_IDENTIFIER:
            return SNUK_STRINGIFY(SNUK_EXPR_IDENTIFIER);
        case SNUK_EXPR_INT:
            return SNUK_STRINGIFY(SNUK_EXPR_INT);
        case SNUK_EXPR_FLOAT:
            return SNUK_STRINGIFY(SNUK_EXPR_FLOAT);
        case SNUK_EXPR_STRING:
            return SNUK_STRINGIFY(SNUK_EXPR_STRING);
        case SNUK_EXPR_BOOL:
            return SNUK_STRINGIFY(SNUK_EXPR_BOOL);
        case SNUK_EXPR_NULL:
            return SNUK_STRINGIFY(SNUK_EXPR_NULL);
        case SNUK_EXPR_UNARY:
            return SNUK_STRINGIFY(SNUK_EXPR_UNARY);
        case SNUK_EXPR_BINARY:
            return SNUK_STRINGIFY(SNUK_EXPR_BINARY);
        case SNUK_EXPR_ASSIGN:
            return SNUK_STRINGIFY(SNUK_EXPR_ASSIGN);
        case SNUK_EXPR_COMPOUND_ASSIGN:
            return SNUK_STRINGIFY(SNUK_EXPR_COMPOUND_ASSIGN);
        case SNUK_EXPR_IF:
            return SNUK_STRINGIFY(SNUK_EXPR_IF);
        case SNUK_EXPR_MATCH:
            return SNUK_STRINGIFY(SNUK_EXPR_MATCH);
        case SNUK_EXPR_WHILE:
            return SNUK_STRINGIFY(SNUK_EXPR_WHILE);
        case SNUK_EXPR_DO_WHILE:
            return SNUK_STRINGIFY(SNUK_EXPR_DO_WHILE);
        case SNUK_EXPR_FOR:
            return SNUK_STRINGIFY(SNUK_EXPR_FOR);
        case SNUK_EXPR_FN:
            return SNUK_STRINGIFY(SNUK_EXPR_FN);
        case SNUK_EXPR_TYPE:
            return SNUK_STRINGIFY(SNUK_EXPR_TYPE);
        case SNUK_EXPR_BLOCK:
            return SNUK_STRINGIFY(SNUK_EXPR_BLOCK);
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
