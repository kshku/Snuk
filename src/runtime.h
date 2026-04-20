#pragma once

#include "defines.h"

#include "memory.h"
#include "interpreter.h"

#include <snmemory/frame.h>

#define PAGES 10

typedef struct Runtime {
    void *mem;
    snFrameAllocator frame;
    SnukInterpreter interpreter;
} Runtime;

SNUK_INLINE Runtime snuk_runtime_init(void) {
    Runtime rt = {0};
    rt.mem = snuk_allocate_pages(PAGES);
    sn_frame_allocator_init(&rt.frame, rt.mem, PAGES * snuk_page_size());
    snuk_interpreter_init(&rt.interpreter);
    return rt;
}

SNUK_INLINE void snuk_runtime_deinit(Runtime *rt) {
    if (!rt) return;
    snuk_free_pages(rt->mem, PAGES);
    sn_frame_allocator_deinit(&rt->frame);
    snuk_interpreter_deinit(&rt->interpreter);
}

bool snuk_runtime_execute(Runtime *rt, const char *src);
