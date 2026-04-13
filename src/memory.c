#include "memory.h"
#include "logger.h"

#include <snmemory/snmemory.h>
#include <stdlib.h>

typedef struct SnukAllocator {
    snFreeListAllocator fla;
    uint32_t total_pages;
    uint32_t committed_pages;
} SnukAllocator;

static bool create_allocator(SnukAllocator *allocator, uint32_t pages);
static void destroy_allocator(SnukAllocator *allocator);
static bool commit_pages(SnukAllocator *allocator, uint32_t pages);

static bool create_allocator(SnukAllocator *allocator, uint32_t pages) {
    void *base = sn_vm_reserve(pages);
    if (!base) return false;

    if (!sn_vm_commit(base, 1)) goto fail;

    *allocator = (SnukAllocator) {
        .total_pages = pages,
        .committed_pages = 1,
    };

    if (!sn_freelist_allocator_init(&allocator->fla, base, sn_vm_get_page_size()))
        goto fail;

    return true;

fail:
    sn_vm_release(base, pages);
    return false;
}

static void destroy_allocator(SnukAllocator *allocator) {
    if (!allocator) return;
    void *base = allocator->fla.mem;
    sn_freelist_allocator_deinit(&allocator->fla);
    sn_vm_decommit(base, allocator->committed_pages);
    sn_vm_release(base, allocator->total_pages);
    *allocator = (SnukAllocator){0};
}

static bool commit_pages(SnukAllocator *allocator, uint32_t pages) {
    if (pages + allocator->committed_pages > allocator->total_pages)
        return false;

    uint8_t *base = allocator->fla.mem;
    if (!sn_vm_commit(base + allocator->committed_pages, pages)) 
        return false;

    allocator->committed_pages += pages;

    // trick to  increase the allocator size
    uint64_t added_size = pages * sn_vm_get_page_size();
    allocator->fla.size += added_size;

    return true;
}

static SnukAllocator gallocator;

bool snuk_memory_init(uint64_t reserve_size) {
    uint32_t pages = (uint32_t)(reserve_size / sn_vm_get_page_size() + 1);
    if (!create_allocator(&gallocator, pages)) return false;
    return true;
}

void snuk_memory_deinit(void) {
    destroy_allocator(&gallocator);
}

void *snuk_alloc(uint64_t size, uint64_t align) {
    void *ptr = sn_freelist_allocator_allocate(&gallocator.fla, size, align);

    if (ptr) return ptr;

    if (!commit_pages(&gallocator, 1)) {
        log_fatal("Ran out of memory!");
        exit(EXIT_FAILURE);
    }

    return snuk_alloc(size, align);

}

void *snuk_realloc(void *ptr, uint64_t new_size, uint64_t align) {
    void *new_ptr = sn_freelist_allocator_reallocate(&gallocator.fla, ptr, new_size, align);
    if (new_ptr) return new_ptr;

    if (!commit_pages(&gallocator, 1)) {
        log_fatal("Ran out of memory!");
        exit(EXIT_FAILURE);
    }

    return snuk_realloc(ptr, new_size, align);
}

void snuk_free(void *ptr) {
    sn_freelist_allocator_free(&gallocator.fla, ptr);
}

