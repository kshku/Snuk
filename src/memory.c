#include "memory.h"
#include "logger.h"

#include <snmemory/snmemory.h>
#include <stdlib.h>

#define KIB(x) ((x) * 1024)
#define MIB(x) (KIB((x) * 1024))
#define GIB(x) (MIB((x) * 1024))

#define SNUK_LINEAR_POOL_SIZE MIB(2)
#define SNUK_STACK_POOL_SIZE MIB(1)
#define SNUK_FREELIST_SIZE MIB(4)

static uint8_t *base;
static uint32_t n_pages;
static snLinearAllocator la;
static snStackAllocator sa;
static snPoolAllocator pa;
static snFreeListAllocator fa;

bool snuk_memory_init(void) {
    uint64_t total = SNUK_LINEAR_POOL_SIZE + SNUK_STACK_POOL_SIZE + SNUK_FREELIST_SIZE;
    n_pages = (uint32_t)((total / sn_vm_get_page_size()) + 1);

    base = (uint8_t *)sn_vm_reserve(n_pages);
    if (!base) return false;

    if (!sn_vm_commit(base, n_pages)) goto fail;

    uint64_t offset = 0;

    if (!sn_linear_allocator_init(&la, base + offset, SNUK_LINEAR_POOL_SIZE)) goto fail;
    offset += SNUK_LINEAR_POOL_SIZE;

    if (!sn_stack_allocator_init(&sa, base + offset, SNUK_STACK_POOL_SIZE)) goto fail;
    offset += SNUK_STACK_POOL_SIZE;

    if (!sn_freelist_allocator_init(&fa, base + offset, SNUK_FREELIST_SIZE)) goto fail;
    offset += SNUK_FREELIST_SIZE;

    // TODO: object allocator

    return true;

fail:
    sn_vm_release(base, n_pages);
    return false;
}

void snuk_memory_deinit(void) {
    // TODO: object allocator
    sn_freelist_allocator_deinit(&fa);
    sn_stack_allocator_deinit(&sa);
    sn_linear_allocator_deinit(&la);

    sn_vm_decommit(base, n_pages);
    sn_vm_release(base, n_pages);
}

void *snuk_alloc(SnukAllocKind kind, uint64_t size, uint64_t align) {
    void *mem = NULL;
    switch (kind) {
        case SNUK_ALLOC_KIND_LINEAR:
            mem = sn_linear_allocator_allocate(&la, size, align);
            break;

        case SNUK_ALLOC_KIND_STACK:
            mem = sn_stack_allocator_allocate(&sa, size, align);
            break;

        case SNUK_ALLOC_KIND_FREELIST:
            mem = sn_freelist_allocator_allocate(&fa, size, align);
            break;

        case SNUK_ALLOC_KIND_POOL:
            // TODO: object allocator
            break;

        default:
            break;
    }

    if (!mem) {
        log_fatal("Failed to allocate memory!");
        exit(EXIT_FAILURE);
    }

    return mem;
}

void snuk_free(SnukAllocKind kind, void *ptr) {
    switch (kind) {
        case SNUK_ALLOC_KIND_LINEAR:
            SNUK_ASSERT(!ptr, "snuk_free on kind SNUK_ALLOC_KIND_LINEAR"
                    "resets the memory and should be called with NULL");
            if (!ptr) sn_linear_allocator_reset(&la);
            break;

        case SNUK_ALLOC_KIND_STACK:
            sn_stack_allocator_free(&sa, ptr);
            break;

        case SNUK_ALLOC_KIND_FREELIST:
            sn_freelist_allocator_free(&fa, ptr);
            break;

        case SNUK_ALLOC_KIND_POOL:
            // TODO: object allocator
            break;

        default:
            break;
    }
}

