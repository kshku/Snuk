#include "snuk_item.h"

#include "snuk_expr.h"
#include "snuk_type.h"
#include "snuk_var.h"

/**
 * @brief Parse an expression item.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed item, or NULL on parse failure.
 */
static SnukItem *parse_expr_item(SnukParser *parser);

/**
 * @brief Parse a variable or constant declaration item.
 *
 * @param parser Parser context to operate on.
 * @param is_const True when parsing a const declaration.
 *
 * @return Parsed item, or NULL on parse failure.
 */
static SnukItem *parse_decl_item(SnukParser *parser, bool is_const);

/**
 * @brief Parse return, break, or continue items.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed item, or NULL on parse failure.
 */
static SnukItem *parse_flow_item(SnukParser *parser);

/**
 * @brief Parse a print item.
 *
 * @param parser Parser context to operate on.
 *
 * @return Parsed item, or NULL on parse failure.
 */
static SnukItem *parse_print_item(SnukParser *parser);

SnukItem *snuk_item_parse(SnukParser *parser) {
    if (parser_match(parser, SNUK_TOKEN_VAR) || parser_match(parser, SNUK_TOKEN_CONST))
        return parse_decl_item(parser, parser->previous.type == SNUK_TOKEN_CONST);

    if (parser_match(parser, SNUK_TOKEN_RETURN) || parser_match(parser, SNUK_TOKEN_CONTINUE)
        || parser_match(parser, SNUK_TOKEN_BREAK))
        return parse_flow_item(parser);

    if (parser_match(parser, SNUK_TOKEN_PRINT)) return parse_print_item(parser);

    return parse_expr_item(parser);
}

static SnukItem *parse_expr_item(SnukParser *parser) {
    SnukExpr *expr = snuk_expr_parse(parser);
    parser_expect_item_end(parser);
    return build_expr_item(parser, expr);
}

static SnukItem *parse_decl_item(SnukParser *parser, bool is_const) {
    SnukVar *var = snuk_var_parse(parser, true);

    parser_expect_item_end(parser);

    return build_decl_item(parser, var, is_const ? SNUK_ITEM_CONST_DECL : SNUK_ITEM_VAR_DECL);
}

static SnukItem *parse_flow_item(SnukParser *parser) {
    SnukTokenType type = parser->previous.type;
    SnukExpr *value = NULL;
    if ((parser->previous.type == SNUK_TOKEN_RETURN || parser->previous.type == SNUK_TOKEN_BREAK)
        && !parser_check_item_end(parser))
        value = snuk_expr_parse(parser);

    parser_expect_item_end(parser);

    return build_flow_item(parser, type, value);
}

static SnukItem *parse_print_item(SnukParser *parser) {
    SnukItem *print_item = build_print_item(parser, NULL, snuk_expr_parse(parser));
    while (parser_match(parser, SNUK_TOKEN_COMMA))
        print_item = build_print_item(parser, print_item, snuk_expr_parse(parser));

    parser_expect_item_end(parser);

    return print_item;
}

const char *snuk_item_type_to_string(SnukItemType type) {
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

void snuk_item_log(SnukItem *item) {
    if (!item) return;
    log_trace("item type: %s", snuk_item_type_to_string(item->type));

    uint64_t count;
    switch (item->type) {
        case SNUK_ITEM_EXPR:
            log_trace("Expr:", NULL);
            snuk_expr_log(item->expr);
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
            snuk_var_log(item->var);
            break;
        case SNUK_ITEM_RETURN:
            log_trace("return:", NULL);
            snuk_expr_log(item->expr);
            break;
        case SNUK_ITEM_BREAK:
            log_trace("break", NULL);
            snuk_expr_log(item->expr);
            break;
        case SNUK_ITEM_CONTINUE:
            log_trace("continue", NULL);
            break;
        case SNUK_ITEM_PRINT:
            log_trace("print:", NULL);
            count = snuk_darray_get_length(item->print_exprs);
            for (uint64_t i = 0; i < count; ++i) snuk_expr_log(item->print_exprs[i]);
            break;
        default:
            break;
    }
}
