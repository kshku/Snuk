#include "snuk/parser/parser.h"

#include "snuk/parser/parser_common.h"
#include "snuk/parser/snuk_item.h"

void snuk_parser_init(SnukParser *parser, const char *src, SnukAllocator *allocator) {
    *parser = (SnukParser){
        .allocator = allocator,
        .panic_mode = false,
    };
    snuk_lexer_init(&parser->lexer, src);

    parser->previous = (SnukToken){0};
    parser->current = snuk_lexer_next_token(&parser->lexer);
    if (parser->current.type == SNUK_TOKEN_ERROR) parser_error(parser, "lexer error");
    parser->next = snuk_lexer_next_token(&parser->lexer);
}

void snuk_parser_deinit(SnukParser *parser) {
    if (!parser) return;
    snuk_lexer_deinit(&parser->lexer);
    *parser = (SnukParser){0};
}

void parser_error(SnukParser *parser, const char *err_msg) {
    if (parser->panic_mode) return;

    parser->panic_mode = true;

    parser->err_msg = err_msg;
    parser->err_token = parser->current;
}

SnukItem *parser_sync(SnukParser *parser) {
    parser->panic_mode = false;
    if (parser->previous.type != SNUK_TOKEN_VSEMICOLON && parser->previous.type != SNUK_TOKEN_SEMICOLON)
        while (!parser_match_item_end(parser)) parser_advance(parser);
    return build_error_item(parser, parser->err_msg, parser->err_token);
}

SnukItem *snuk_parser_next_item(SnukParser *parser) {
    if (parser->current.type == SNUK_TOKEN_EOF) return NULL;
    SnukItem *item = snuk_item_parse(parser);
    if (parser->panic_mode) return parser_sync(parser);
    return item;
}
