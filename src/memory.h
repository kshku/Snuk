#pragma once

#include "defines.h"

#define KIB(x) ((x) * 1024)
#define MIB(x) (KIB((x) * 1024))
#define GIB(x) (MIB((x) * 1024))

bool snuk_memory_init(uint64_t reserve_size);
void snuk_memory_deinit(void);

// should be freed in stack order, first allocated should be freed last
void *snuk_allocate_pages(uint32_t pages);
void snuk_free_pages(void *base, uint32_t pages);

void *snuk_alloc(uint64_t size, uint64_t align);
void *snuk_realloc(void *ptr, uint64_t new_size, uint64_t align);
void snuk_free(void *ptr);
