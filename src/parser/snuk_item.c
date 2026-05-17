#include "snuk_item.h"

#include "snuk_expr.h"
#include "snuk_type.h"

/**
 * @brief Parse an expression item.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed item, or NULL on parse failure.
 */
static SnukItem *parse_expr_item(SnukParser *parser, ParseFlag flag);

/**
 * @brief Parse a variable or constant declaration item.
 *
 * @param parser Parser context to operate on.
 * @param is_const True when parsing a const declaration.
 *
 * @return Parsed item, or NULL on parse failure.
 */
static SnukItem *parse_decl_item(
    SnukParser *parser, bool is_const, ParseFlag flag);

/**
 * @brief Parse return, break, or continue items.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed item, or NULL on parse failure.
 */
static SnukItem *parse_flow_item(SnukParser *parser, ParseFlag flag);

/**
 * @brief Parse a print item.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed item, or NULL on parse failure.
 */
static SnukItem *parse_print_item(SnukParser *parser, ParseFlag flag);

SnukItem *parse_item(SnukParser *parser, ParseFlag flag) {
    if (parser_match(parser, SNUK_TOKEN_VAR)
        || parser_match(parser, SNUK_TOKEN_CONST))
        return parse_decl_item(
            parser, parser->previous.type == SNUK_TOKEN_CONST, flag);

    if (parser_match(parser, SNUK_TOKEN_RETURN)
        || parser_match(parser, SNUK_TOKEN_CONTINUE)
        || parser_match(parser, SNUK_TOKEN_BREAK))
        return parse_flow_item(parser, flag);

    if (parser_match(parser, SNUK_TOKEN_PRINT))
        return parse_print_item(parser, flag);

    return parse_expr_item(parser, flag);
}

static SnukItem *parse_expr_item(SnukParser *parser, ParseFlag flag) {
    SnukExpr *expr = parse_expression(parser, flag);
    parser_expect_item_end(parser);
    return build_expr_item(parser, expr);
}

static SnukItem *parse_decl_item(
    SnukParser *parser, bool is_const, ParseFlag flag) {
    parser_expect(parser, SNUK_TOKEN_IDENTIFIER, "expected an identifier");
    SnukStringView identifier = parser->previous.string_literal;

    SnukType *type = NULL;
    if (parser_match(parser, SNUK_TOKEN_COLON))
        type = parse_type_annot(parser, flag);
    else type = build_any_type(parser);

    SnukExpr *init = NULL;
    if (parser_match(parser, SNUK_TOKEN_ASSIGN))
        init = parse_expression(parser, flag);
    else init = build_null_expr(parser);

    parser_expect_item_end(parser);

    return build_decl_item(
        parser, identifier, type, init,
        is_const ? SNUK_ITEM_CONST_DECL : SNUK_ITEM_VAR_DECL);
}

static SnukItem *parse_flow_item(SnukParser *parser, ParseFlag flag) {
    SnukTokenType type = parser->previous.type;
    SnukExpr *value = NULL;
    if (parser->previous.type == SNUK_TOKEN_RETURN
        || parser->previous.type == SNUK_TOKEN_BREAK)
        // TODO: look for delimiter, value is optional
        // TODO: break and return items should be at the end of block only?
        value = parse_expression(parser, flag);

    parser_expect_item_end(parser);

    return build_flow_item(parser, type, value);
}

static SnukItem *parse_print_item(SnukParser *parser, ParseFlag flag) {
    SnukItem *print_item =
        build_print_item(parser, NULL, parse_expression(parser, flag));
    while (parser_match(parser, SNUK_TOKEN_COMMA))
        print_item = build_print_item(
            parser, print_item, parse_expression(parser, flag));

    parser_expect_item_end(parser);

    return print_item;
}

const char *snuk_parser_item_type_to_string(SnukItemType type) {
    switch (type) {
        case SNUK_ITEM_EXPR:
            return SNUK_STRINGIFY(SNUK_ITEM_EXPR);
        case SNUK_ITEM_VAR_DECL:
            return SNUK_STRINGIFY(SNUK_ITEM_VAR_DECL);
        case SNUK_ITEM_CONST_DECL:
            return SNUK_STRINGIFY(SNUK_ITEM_CONST_DECL);
        case SNUK_ITEM_PRINT:
            return SNUK_STRINGIFY(SNUK_ITEM_PRINT);
        case SNUK_ITEM_RETURN:
            return SNUK_STRINGIFY(SNUK_ITEM_RETURN);
        case SNUK_ITEM_BREAK:
            return SNUK_STRINGIFY(SNUK_ITEM_BREAK);
        case SNUK_ITEM_CONTINUE:
            return SNUK_STRINGIFY(SNUK_ITEM_CONTINUE);
        case SNUK_ITEM_MAX:
            return SNUK_STRINGIFY(SNUK_ITEM_MAX);
        default:
            return "Unkown item type";
    }
}

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
        case SNUK_ITEM_CONST_DECL:
            switch (item->type) {
                case SNUK_ITEM_VAR_DECL:
                    log_trace("var:", NULL);
                    break;
                case SNUK_ITEM_CONST_DECL:
                    log_trace("const:", NULL);
                    break;
                default:
                    break;
            }
            log_trace(
                "identifier: " SNUK_STRING_VIEW_FORMAT,
                SNUK_STRING_VIEW_ARG(item->decl_item.name));
            if (item->decl_item.type) log_trace("type: ", NULL);
            snuk_parser_log_type(item->decl_item.type);
            snuk_parser_log_expr(item->decl_item.expr);
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
        case SNUK_ITEM_PRINT:
            log_trace("print:", NULL);
            count = snuk_darray_get_length(item->print_exprs);
            for (uint64_t i = 0; i < count; ++i)
                snuk_parser_log_expr(item->print_exprs[i]);
            break;
        default:
            break;
    }
}
