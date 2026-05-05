#include "runtime.h"

#include "parser.h"
#include "logger.h"

void snuk_runtime_execute(Runtime *rt, const char *src) {
    SnukParser parser;
    snuk_parser_init(&parser, src, &rt->parser_allocator);

    SnukItem *item;
    while (true) {
        item = snuk_parser_next_item(&parser);
        if (!item) break;
        // snuk_parser_log_item(item);
        // log_trace("", NULL);
        SnukValue value = snuk_interpreter_exec_item(&rt->interpreter, item);
        SNUK_UNUSED(value);
        snuk_interpreter_log_value(value);
        log_trace("", NULL);
    }

    snuk_parser_deinit(&parser);
}
