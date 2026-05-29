#include "snuk/parser/snuk_type.h"

#include "snuk/parser/snuk_var.h"

SnukType any_type = {
    .type = TYPE_ANY,
};

SnukType type_type = {
    .type = TYPE_TYPE,
};

SnukType *snuk_type_parse_interface(SnukParser *parser) {
    SnukType *type = build_interface_type(parser, NULL, NULL);
    parser_expect(parser, SNUK_TOKEN_LBRACE, "expected '{'");

    while (!parser_match(parser, SNUK_TOKEN_RBRACE) && parser->current.type != SNUK_TOKEN_EOF) {
        if (!parser_match(parser, SNUK_TOKEN_VAR) && !parser_match(parser, SNUK_TOKEN_CONST)) {
            parser_error(parser, "expected var or const");
            return NULL;
        }

        SnukVar *var = snuk_var_parse(parser, false);
        parser_expect_item_end(parser);
        if (var->value) parser_error(parser, "interface members should not have values");
        type = build_interface_type(parser, type, var);
    }

    if (parser->previous.type != SNUK_TOKEN_RBRACE) {
        parser_error(parser, "expected '}'");
        return NULL;
    }

    return type;
}

SnukType *snuk_type_parse(SnukParser *parser) {
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

    if (parser_match(parser, SNUK_TOKEN_INTERFACE)) return snuk_type_parse_interface(parser);

    if (parser_match(parser, SNUK_TOKEN_TYPE)) return build_type_type(parser);
    if (parser_match(parser, SNUK_TOKEN_ANY)) return build_any_type(parser);

    parser_expect(parser, SNUK_TOKEN_IDENTIFIER, "expected a type name");

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

bool snuk_type_equal(SnukType *type1, SnukType *type2) {
    if (type1->type != type2->type) return false;

    uint64_t count1;
    uint64_t count2;
    switch (type1->type) {
        case TYPE_ANY:
        case TYPE_TYPE:
            return true;

        case TYPE_NAMED:
            return snuk_string_view_equal(type1->name, type2->name);

        case TYPE_FN:
            if (!snuk_type_equal(type1->fn.return_type, type2->fn.return_type)) return false;

            count1 = snuk_darray_get_length(type1->fn.param_types);
            count2 = snuk_darray_get_length(type2->fn.param_types);
            if (count1 != count2) return false;

            for (uint64_t i = 0; i < count1; ++i)
                if (!snuk_type_equal(type1->fn.param_types[i], type2->fn.param_types[i]))
                    return false;
            return true;

        case TYPE_INTERFACE:
            count1 = snuk_darray_get_length(type1->members);
            count2 = snuk_darray_get_length(type2->members);
            if (count1 != count2) return false;
            for (uint64_t i = 0; i < count1; ++i) {
                if (!snuk_string_view_equal(type1->members[i]->name, type2->members[i]->name))
                    return false;
                if (!snuk_type_equal(type1->members[i]->type, type2->members[i]->type))
                    return false;
            }

            return true;

        default:
            SNUK_SHOULD_NOT_REACH_HERE;
            break;
    }
    return false;
}
