#include "parser.h"

#include "io.h"
#include "parser_common.h"

SnukItem *snuk_parser_next_item(SnukParser *parser) {
    if (parser->current.type == SNUK_TOKEN_EOF) return NULL;
    SnukItem *item = parse_item(parser, PARSE_FLAG_NORMAL);
    if (parser->panic_mode) parser_sync(parser);
    return item;
}

void parser_error(SnukParser *parser, const char *err_msg) {
    if (parser->panic_mode) return;

    parser->panic_mode = true;
    parser->had_error = true;

    SnukToken t = parser->current;
    snuk_eprint("%lu:%lu error", t.line, t.col);
    if (t.type == SNUK_TOKEN_EOF) snuk_eprint(" at end");
    else
        snuk_eprint(
            " at '" SNUK_STRING_VIEW_FORMAT "'",
            SNUK_STRING_VIEW_ARG(t.string_literal));
    snuk_eprintln(" message: '%s'", err_msg);
}

void parser_sync(SnukParser *parser) {
    // TODO: skipping errors
    parser->panic_mode = false;
    return;

    while (parser->current.type != SNUK_TOKEN_EOF) {
        if (parser->previous.type == SNUK_TOKEN_SEMICOLON) return;

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
