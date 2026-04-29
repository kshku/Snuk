#include "runtime.h"

#include "parser.h"
#include "io.h"
#include "snuk_string.h"
#include "logger.h"

bool snuk_runtime_execute(Runtime *rt, const char *src) {
    if (snuk_string_equal(src, "exit\n")) return true;

    SnukParser parser;
    snuk_parser_init(&parser, src, (void *)(&rt->frame), (alloc_fn)sn_frame_allocator_allocate);

    SnukItem *item;
    while (true) {
        sn_frame_allocator_begin(&rt->frame);
        item = snuk_parser_next_item(&parser);
        if (!item) break;
        snuk_parser_log_item(item);
        log_trace("", NULL);
        SnukValue value = snuk_interpreter_exec_item(&rt->interpreter, item);
        snuk_interpreter_print_value(value);
        snuk_println("");
        sn_frame_allocator_end(&rt->frame);
    }

    snuk_parser_deinit(&parser);

    return false;
}
