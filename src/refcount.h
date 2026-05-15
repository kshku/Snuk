#pragma once

#include "defines.h"

#include "memory.h"

#include "logger.h"

// free_fn must free `mem` and any owned resources.
// It must NOT free the refcounter itself.
typedef void (*SnukRefCounterFreeFn)(void *data, void *ptr);

typedef struct SnukRefCounter {
    void *mem;
    uint64_t ref_count;

    void *data;
    SnukRefCounterFreeFn free_fn;
} SnukRefCounter;

SNUK_INLINE SnukRefCounter *snuk_ref_counter_create(void *mem, void *data, SnukRefCounterFreeFn free_fn) {
    SnukRefCounter *rc = (SnukRefCounter *)snuk_alloc(sizeof(SnukRefCounter), alignof(SnukRefCounter));
    *rc = (SnukRefCounter){
        .mem = mem,
        .ref_count = 1,

        .data = data,
        .free_fn = free_fn,
    };
    log_debug("created a ref counter", NULL);
    return rc;
}

SNUK_INLINE SnukRefCounter *snuk_ref_counter_retain(SnukRefCounter *rc) {
    SNUK_ASSERT(rc, "SnukRefCounter is null");
    log_debug("retained a ref counter", NULL);
    rc->ref_count++;
    return rc;
}

SNUK_INLINE void *snuk_ref_counter_get(SnukRefCounter *rc) {
    SNUK_ASSERT(rc, "SnukRefCounter is null");
    return rc->mem;
}

SNUK_INLINE void snuk_ref_counter_release(SnukRefCounter **rc) {
    SNUK_ASSERT(*rc, "SnukRefCounter is null");
    log_debug("released a ref counter", NULL);
    (*rc)->ref_count--;

    if ((*rc)->ref_count == 0) {
        (*rc)->free_fn((*rc)->data, (*rc)->mem);
        snuk_free(*rc);
        log_debug("a ref counter got destroyed", NULL);
    }

    *rc = NULL;
}

SNUK_FORCE_INLINE SnukRefCounter *snuk_ref_counter_move(SnukRefCounter **rc) {
    SnukRefCounter *tmp = *rc;
    *rc = NULL;
    return tmp;
}
