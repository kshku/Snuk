#include "snuk/parser/snuk_var.h"

#include "snuk/parser/snuk_expr.h"
#include "snuk/parser/snuk_type.h"

SnukVar *snuk_var_parse(SnukParser *parser, bool default_null) {
    parser_expect(parser, SNUK_TOKEN_IDENTIFIER, "expected an identifier");

    SnukStringView name = parser->previous.string_literal;

    SnukType *type;
    if (parser_match(parser, SNUK_TOKEN_COLON)) type = snuk_type_parse(parser);
    else type = build_any_type(parser);

    SnukExpr *value;
    if (parser_match(parser, SNUK_TOKEN_ASSIGN)) value = snuk_expr_parse(parser);
    else if (default_null) value = build_null_expr(parser);
    else value = NULL;

    return build_var(parser, name, type, value);
}

void snuk_var_log(SnukVar *var) {
    if (!var) return;
    log_trace("var " SNUK_STRING_VIEW_FORMAT, SNUK_STRING_VIEW_ARG(var->name));
    snuk_type_log(var->type);
    snuk_expr_log(var->value);
}
