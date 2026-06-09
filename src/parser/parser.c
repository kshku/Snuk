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
    if (parser->current.type == SNUK_TOKEN_ERROR)
        parser_error(parser, SNUK_PARSE_ERR_LEXER_ERROR, "lexer error");
    parser->next = snuk_lexer_next_token(&parser->lexer);
}

SnukError snuk_parser_clear_error(SnukParser *parser) {
    SnukError err = parser->err;
    parser->err = SNUK_ERROR_NONE;
    return err;
}

void snuk_parser_deinit(SnukParser *parser) {
    if (!parser) return;
    snuk_lexer_deinit(&parser->lexer);
    *parser = (SnukParser){0};
}

void parser_error(SnukParser *parser, SnukParseError code, const char *msg) {
    if (parser->panic_mode) return;

    parser->panic_mode = true;

    parser->err = (SnukError){
        .kind = SNUK_ERROR_KIND_PARSE,
        .code = code,
        .msg = msg,
        .loc = {
            .line = parser->current.line,
            .col = parser->current.col,
        },
    };
}

SnukItem *parser_sync(SnukParser *parser) {
    while (parser->current.type != SNUK_TOKEN_EOF && parser->current.type != SNUK_TOKEN_SEMICOLON
           && parser->current.type != SNUK_TOKEN_VSEMICOLON) {
        parser_advance(parser);
    }
    SnukItem *item = build_error_item(parser, parser->err);
    parser->panic_mode = false;
    return item;
}

SnukItem *snuk_parser_next_item(SnukParser *parser) {
    if (parser->current.type == SNUK_TOKEN_EOF) return NULL;
    SnukItem *item = snuk_item_parse(parser);
    if (parser->panic_mode) return parser_sync(parser);
    return item;
}
