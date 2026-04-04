#pragma once

#include "defines.h"

typedef enum SnukAllocKind {
    SNUK_ALLOC_KIND_LINEAR,
    SNUK_ALLOC_KIND_FREELIST,
    SNUK_ALLOC_KIND_POOL,
    SNUK_ALLOC_KIND_STACK,
} SnukAllocKind;

bool snuk_memory_init(void);
void snuk_memory_deinit(void);

void *snuk_alloc(SnukAllocKind kind, uint64_t size, uint64_t align);
void snuk_free(SnukAllocKind kind, void *ptr);

