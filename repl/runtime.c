#include "runtime.h"

#include <snuk/logger.h>
#include <snuk/parser/parser.h>
#include <snuk/snuk_error.h>

void snuk_runtime_execute(Runtime *rt, const char *src) {
    SnukParser parser;
    snuk_parser_init(&parser, src, &rt->parser_allocator);

    SnukItem *item;
    while (true) {
        item = snuk_parser_next_item(&parser);
        if (!item) break;

        SnukError parse_err = snuk_parser_clear_error(&parser);
        if (parse_err.kind != SNUK_ERROR_KIND_NONE) {
            log_error("[Error] at line %u, col %u: %s", parse_err.loc.line, parse_err.loc.col, parse_err.msg);
            continue;
        }

        SnukValue value = snuk_interpreter_exec_item(&rt->interpreter, item);
        SnukError err = snuk_interpreter_clear_error(&rt->interpreter);
        if (err.kind != SNUK_ERROR_KIND_NONE) {
            if (err.loc.line)
                log_error("[Error] at line %u, col %u: %s", err.loc.line, err.loc.col, err.msg);
            else
                log_error("[Error] %s", err.msg);
        }
        snuk_value_log(value);
        log_trace("", NULL);
        snuk_value_free(value);
    }

    snuk_parser_deinit(&parser);
}
