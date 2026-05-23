#pragma once

#include "defines.h"
#include "logger.h"
#include "memory.h"

// free_fn must free `mem` and any owned resources.
// It must NOT free the refcounter itself.
typedef void (*SnukRefCounterFreeFn)(void *data, void *ptr);

typedef struct SnukRefCounter {
    void *mem;
    uint64_t strong_count;
    uint64_t weak_count;

    void *data;
    SnukRefCounterFreeFn free_fn;
} SnukRefCounter;

SNUK_INLINE SnukRefCounter *snuk_ref_counter_create(void *mem, void *data, SnukRefCounterFreeFn free_fn) {
    SnukRefCounter *rc = (SnukRefCounter *)snuk_alloc(sizeof(SnukRefCounter), alignof(SnukRefCounter));
    *rc = (SnukRefCounter){
        .mem = mem,
        .strong_count = 1,
        .weak_count = 0,

        .data = data,
        .free_fn = free_fn,
    };
    log_debug("created a ref counter", NULL);
    return rc;
}

SNUK_INLINE void *snuk_ref_counter_get(SnukRefCounter *rc) {
    if (!rc) return NULL;
    return rc->mem;
}

SNUK_INLINE SnukRefCounter *snuk_ref_counter_retain(SnukRefCounter *rc) {
    SNUK_ASSERT(rc, "SnukRefCounter is null");
    if (!rc->strong_count) return NULL;
    log_debug("retained a strong ref counter", NULL);
    rc->strong_count++;
    return rc;
}

SNUK_INLINE SnukRefCounter *snuk_ref_counter_retain_weak(SnukRefCounter *rc) {
    SNUK_ASSERT(rc, "SnukRefCounter is null");
    if (!rc->weak_count && !rc->strong_count) return NULL;
    log_debug("retained a weak ref counter", NULL);
    rc->weak_count++;
    return rc;
}

SNUK_INLINE void snuk_ref_counter_release(SnukRefCounter **rc) {
    SNUK_ASSERT(*rc, "SnukRefCounter is null");
    SNUK_ASSERT((*rc)->strong_count, "releasing strong reference when no strong reference exists");

    log_debug("released a strong ref counter", NULL);
    (*rc)->strong_count--;

    if ((*rc)->strong_count == 0) {
        (*rc)->free_fn((*rc)->data, (*rc)->mem);
        (*rc)->mem = NULL;
        log_debug("ref counter held memory got destroyed", NULL);
    }

    if ((*rc)->strong_count + (*rc)->weak_count == 0) {
        snuk_free(*rc);
        log_debug("a ref counter got destroyed", NULL);
    }

    *rc = NULL;
}

SNUK_INLINE void snuk_ref_counter_release_weak(SnukRefCounter **rc) {
    SNUK_ASSERT(*rc, "SnukRefCounter is null");
    SNUK_ASSERT((*rc)->weak_count, "releasing weak reference when no weak reference exists");

    log_debug("released a weak ref counter", NULL);
    (*rc)->weak_count--;

    if ((*rc)->strong_count + (*rc)->weak_count == 0) {
        snuk_free(*rc);
        log_debug("a ref counter got destroyed", NULL);
    }

    *rc = NULL;
}

SNUK_INLINE void snuk_ref_counter_downgrade(SnukRefCounter *rc) {
    SNUK_ASSERT(rc, "SnukRefCounter is null");
    SNUK_UNUSED(snuk_ref_counter_retain_weak(rc));
    snuk_ref_counter_release(&rc);
}

SNUK_FORCE_INLINE SnukRefCounter *snuk_ref_counter_move(SnukRefCounter **rc) {
    SnukRefCounter *tmp = *rc;
    *rc = NULL;
    return tmp;
}
