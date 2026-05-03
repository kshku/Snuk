#pragma once

#include "defines.h"

#include "memory.h"
#include "interpreter.h"
#include "snuk_string.h"

#include <snmemory/frame.h>

#define PAGES 10

typedef struct Runtime {
    void *mem;
    snLinearAllocator la;
    SnukInterpreter interpreter;
} Runtime;

SNUK_INLINE void snuk_runtime_init(Runtime *rt) {
    *rt = (Runtime){
        .mem = snuk_allocate_pages(PAGES),
        .parser_allocator = {
            .data = (void *)&rt->la,
            .alloc = runtime_alloc_fn,
            .realloc = runtime_realloc_fn,
            .free = runtime_free_fn,
        },
    };
    sn_linear_allocator_init(&rt->la, rt->mem, PAGES * snuk_page_size());
    snuk_interpreter_init(&rt->interpreter);
}

SNUK_INLINE void snuk_runtime_deinit(Runtime *rt) {
    if (!rt) return;
    snuk_interpreter_deinit(&rt->interpreter);
    sn_linear_allocator_deinit(&rt->la);
    snuk_free_pages(rt->mem, PAGES);
    *rt = (Runtime){0};
}

void snuk_runtime_execute(Runtime *rt, const char *src);

SNUK_INLINE void snuk_runtime_execute_file(Runtime *rt, const char *src) {
    snuk_runtime_execute(rt, src);
}

SNUK_INLINE bool snuk_runtime_execute_repl(Runtime *rt, const char *src) {
    if (snuk_string_n_equal(src, "exit", 4)) return true;

    snuk_runtime_execute(rt, src);

    return false;
}
