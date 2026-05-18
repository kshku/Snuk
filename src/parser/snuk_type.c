#include "snuk_type.h"

SnukType *snuk_type_parse(SnukParser *parser) {
    if (parser_match(parser, SNUK_TOKEN_TYPE)) {
        // type <type>
        if (parser_match(parser, SNUK_TOKEN_IDENTIFIER))
            return build_named_type(parser, parser->previous.string_literal);

        // type {<type>; <type>}
        SnukType *type = build_type_type(parser, NULL, NULL);
        parser_expect(parser, SNUK_TOKEN_LBRACE, "expected '{'");
        while (!parser_match(parser, SNUK_TOKEN_RBRACE) && parser->current.type != SNUK_TOKEN_EOF) {
            SnukType *member_type = snuk_type_parse(parser);
            type = build_type_type(parser, type, member_type);
            if (!parser_check(parser, SNUK_TOKEN_RBRACE)) parser_expect_item_end(parser);
        }

        if (parser->previous.type != SNUK_TOKEN_RBRACE) {
            parser_error(parser, "expected '}'");
            return NULL;
        }

        return type;
    }

    if (parser_match(parser, SNUK_TOKEN_FN)) {
        SnukType *type = build_fn_type(parser, NULL, NULL, NULL);
        parser_expect(parser, SNUK_TOKEN_LPAREN, "exptected '('");
        while (!parser_match(parser, SNUK_TOKEN_RPAREN) && parser->current.type != SNUK_TOKEN_EOF) {
            SnukType *param = snuk_type_parse(parser);
            type = build_fn_type(parser, type, param, NULL);
            if (!parser_check(parser, SNUK_TOKEN_RPAREN))
                parser_expect(parser, SNUK_TOKEN_COMMA, "expected ','");
        }

        if (parser->previous.type != SNUK_TOKEN_RPAREN) {
            parser_error(parser, "expected ')'");
            return NULL;
        }

        SnukType *ret_type = NULL;
        if (parser_match(parser, SNUK_TOKEN_ARROW)) ret_type = snuk_type_parse(parser);
        else ret_type = build_any_type(parser);

        type = build_fn_type(parser, type, NULL, ret_type);
        return type;
    }

    if (parser_match(parser, SNUK_TOKEN_ANY)) return build_any_type(parser);

    parser_expect(parser, SNUK_TOKEN_IDENTIFIER, "unexpected type");

    return build_named_type(parser, parser->previous.string_literal);
}

void snuk_type_log(SnukType *type) {
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
            log_trace("type name: " SNUK_STRING_VIEW_FORMAT, SNUK_STRING_VIEW_ARG(type->name));
            break;
        case TYPE_FN:
            log_trace("type type: %s", SNUK_STRINGIFY(TYPE_FN));
            log_trace("param types:", NULL);
            count = snuk_darray_get_length(type->fn.param_types);
            for (uint64_t i = 0; i < count; ++i) snuk_type_log(type->fn.param_types[i]);
            log_trace("return type:", NULL);
            snuk_type_log(type->fn.return_type);
            break;
        default:
            break;
    }
}
