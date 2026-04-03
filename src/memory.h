#pragma once

#include "defines.h"

typedef enum SnukAllocKind {
    SNUK_ALLOC_KIND_TEMP,
    SNUK_ALLOC_KIND_STRING,
    SNUK_ALLOC_KIND_OBJECT,
    SNUK_ALLOC_KIND_FRAME,
} SnukAllocKind;

bool snuk_memory_init(void);
void snuk_memory_deinit(void);

void *snuk_alloc(SnukAllocKind kind, uint64_t size, uint64_t align);
void snuk_free(SnukAllocKind kind, void *ptr);

