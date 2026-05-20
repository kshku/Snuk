#include "memory.h"

#include "logger.h"

#include <stdlib.h>

typedef struct SnukPageAllocator {
    uint8_t *base;
    uint32_t total_pages;
    uint32_t committed_pages;
    uint32_t reverse_committed_pages;
} SnukPageAllocator;

static bool create_allocator(SnukPageAllocator *allocator, uint32_t pages);
static void destroy_allocator(SnukPageAllocator *allocator);
static void *commit_pages(SnukPageAllocator *allocator, uint32_t pages);
static void *reverse_commit_pages(SnukPageAllocator *allocator, uint32_t pages);
static void reverse_decommit_pages(SnukPageAllocator *allocator, void *base, uint32_t pages);

static void try_increasing_allocator_size(void);

static SnukPageAllocator page_allocator;
static snFreeListAllocator galloc;

bool snuk_memory_init(uint64_t reserve_size) {
    uint64_t page_size = sn_vm_get_page_size();
    uint32_t pages = (uint32_t)(reserve_size / page_size + 1);
    if (!create_allocator(&page_allocator, pages)) return false;

    void *base = commit_pages(&page_allocator, 1);
    if (!base) return false;

    if (!sn_freelist_allocator_init(&galloc, base, page_size)) return false;

    return true;
}

void snuk_memory_deinit(void) {
    sn_freelist_allocator_deinit(&galloc);
    destroy_allocator(&page_allocator);
}

void *snuk_allocate_pages(uint32_t pages) {
    void *base = reverse_commit_pages(&page_allocator, pages);
    if (!base) {
        log_fatal("Ran out of memory!", NULL);
        exit(EXIT_FAILURE);
    }
    return base;
}

void snuk_free_pages(void *base, uint32_t pages) {
    reverse_decommit_pages(&page_allocator, base, pages);
}

void *snuk_alloc(uint64_t size, uint64_t align) {
    void *ptr = sn_freelist_allocator_allocate(&galloc, size, align);

    if (ptr) return ptr;
    try_increasing_allocator_size();
    return snuk_alloc(size, align);
}

void *snuk_realloc(void *ptr, uint64_t new_size, uint64_t align) {
    void *new_ptr = sn_freelist_allocator_reallocate(&galloc, ptr, new_size, align);
    if (new_ptr) return new_ptr;
    try_increasing_allocator_size();
    return snuk_realloc(ptr, new_size, align);
}

void snuk_free(void *ptr) {
    sn_freelist_allocator_free(&galloc, ptr);
}

static bool create_allocator(SnukPageAllocator *allocator, uint32_t pages) {
    *allocator = (SnukPageAllocator){
        .base = sn_vm_reserve(pages),
        .total_pages = pages,
        .committed_pages = 0,
        .reverse_committed_pages = 0,
    };

    if (!allocator->base) return false;

    return true;
}

static void destroy_allocator(SnukPageAllocator *allocator) {
    if (!allocator) return;
    sn_vm_decommit(allocator->base, allocator->committed_pages);
    sn_vm_decommit(allocator->base, allocator->total_pages);
    *allocator = (SnukPageAllocator){0};
}

static void *commit_pages(SnukPageAllocator *allocator, uint32_t pages) {
    if (pages + allocator->committed_pages + allocator->reverse_committed_pages > allocator->total_pages)
        return NULL;

    uint64_t page_size = sn_vm_get_page_size();
    void *base = allocator->base + (allocator->committed_pages * page_size);
    if (!sn_vm_commit(base, pages)) return NULL;

    allocator->committed_pages += pages;

    return base;
}

static void *reverse_commit_pages(SnukPageAllocator *allocator, uint32_t pages) {
    if (pages + allocator->committed_pages + allocator->reverse_committed_pages > allocator->total_pages)
        return NULL;

    uint64_t page_size = sn_vm_get_page_size();
    uint8_t *base
        = allocator->base + ((allocator->total_pages - allocator->reverse_committed_pages - pages) * page_size);
    if (!sn_vm_commit(base, pages)) return NULL;

    allocator->reverse_committed_pages += pages;

    return base;
}

static void reverse_decommit_pages(SnukPageAllocator *allocator, void *base, uint32_t pages) {
    uint64_t page_size = sn_vm_get_page_size();
    void *actual_base
        = allocator->base + ((allocator->total_pages - allocator->reverse_committed_pages) * page_size);
    SNUK_ASSERT(actual_base == base, "freeing in non-stack order");
    sn_vm_decommit(base, pages);
    allocator->reverse_committed_pages -= pages;
}

static void try_increasing_allocator_size(void) {
    void *base = commit_pages(&page_allocator, 1);
    if (!base) {
        log_fatal("Ran out of memory!", NULL);
        exit(EXIT_FAILURE);
    }

    sn_freelist_allocator_increase_memory_size(&galloc, base, sn_vm_get_page_size());
}

SNUK_INLINE void *snuk_allocator_global_alloc(void *data, uint64_t size, uint64_t align) {
    SNUK_UNUSED(data);
    return snuk_alloc(size, align);
}

SNUK_INLINE void snuk_allocator_global_free(void *data, void *ptr) {
    SNUK_UNUSED(data);
    snuk_free(ptr);
}

SNUK_INLINE void *snuk_allocator_global_realloc(void *data, void *ptr, uint64_t new_size, uint64_t align) {
    SNUK_UNUSED(data);
    return snuk_realloc(ptr, new_size, align);
}

SnukAllocator snuk_global_allocator = {
    .data = NULL,
    .alloc = snuk_allocator_global_alloc,
    .realloc = snuk_allocator_global_realloc,
    .free = snuk_allocator_global_free,
};
