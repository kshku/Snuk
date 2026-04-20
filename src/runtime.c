#include "runtime.h"

#include "parser.h"
#include "io.h"
#include "snuk_string.h"
#include "logger.h"

bool snuk_runtime_execute(Runtime *rt, const char *src) {
    if (string_equal(src, "exit\n")) return true;

    SnukParser parser;
    snuk_parser_init(&parser, src, (void *)(&rt->frame), (alloc_fn)sn_frame_allocator_allocate);

    SnukStmt *stmt;
    while ((stmt = snuk_parser_next_stmt(&parser))) {
        sn_frame_allocator_begin(&rt->frame);
        snuk_parser_log_stmt(stmt);
        log_trace("", NULL);
        snuk_interpreter_exec_stmt(&rt->interpreter, stmt);
        sn_frame_allocator_end(&rt->frame);
    }

    snuk_parser_deinit(&parser);

    return false;
}
