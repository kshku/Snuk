#include "memory.h"

#include "core/logger.h"
#include "platform/memory.h"

// TODO: Might want to change how things work here.
// TODO: If called before initialising then print a warning(or may be an info)
// TODO: and call allocation or deallocation functions instead of just returning
// TODO: NULL.

typedef struct PtrSizePair {
        u64 size;
        void *ptr;
} PtrSizePair;

typedef struct MemState {
        b8 initialized;
        u64 total_allocated;
        u32 index;
        u64 size;
        PtrSizePair *allocated_ptrs;
} MemState;

static MemState mem_state;

/**
 * @brief Initialize the memory subsystem.
 *
 * @return true memory subsystem was initialized successfully.
 */
b8 initializeMemory() {
    if (mem_state.initialized) {
        SERROR("Memory system is already initialized, but initializeMemory was "
               "called again");
        return false;
    }

    mem_state.total_allocated = 0;
    mem_state.index = 0;
    mem_state.size = 1;
    mem_state.allocated_ptrs =
        platformAllocateMemory(mem_state.size * sizeof(PtrSizePair));
    mem_state.initialized = true;

    return true;
}

/**
 * @brief Shutdown the memory subsystem.
 */
void shutdownMemory() {
    if (!mem_state.initialized) {
        STRACE("shutdownMemory called without initializing Memory");
        return;
    }

    if (mem_state.allocated_ptrs) {
        for (u32 i = 0; i < mem_state.index; ++i) {
            SWARN("Allocated %ld bytes of memory was not deallocated, "
                  "Deallocating...",
                  mem_state.allocated_ptrs[i].size);
            platformDeallocateMemory(mem_state.allocated_ptrs[i].ptr);
        }
        platformDeallocateMemory(mem_state.allocated_ptrs);
        mem_state.allocated_ptrs = NULL;
    }
    mem_state.initialized = false;
}

/**
 * @brief (INTERNAL FUNCTION) update allocated_ptrs
 *
 * @param ptr Pointer to allocate
 * @param size Allocated size if allocation, else fill the deallocation size
 * @param is_allocation true if allocation, false if deallocation
 *
 * @return true on success else false.
 */
b8 updateAllocatedPtrs(void *ptr, u64 *size, b8 is_allocation) {
    if (!mem_state.initialized) {
        // TODO: Write assertion
        SERROR("updateAllocatedPtrs called before initializing the memory");
        return false;
    }

    if (is_allocation) {
        if (mem_state.size == mem_state.index) {
            mem_state.size += 2;
            void *ptr = platformReallocateMemory(
                mem_state.allocated_ptrs,
                (mem_state.size * sizeof(PtrSizePair)));
            if (!ptr) {
                SERROR("updateAllocatedPtrs reallocation failed");
                mem_state.size -= 2;
                return false;
            }
            mem_state.allocated_ptrs = (PtrSizePair *)ptr;
        }
        mem_state.allocated_ptrs[mem_state.index++] =
            (PtrSizePair){.ptr = ptr, .size = (*size)};
        return true;
    }

    for (u32 i = 0; i < mem_state.index; ++i) {
        if (mem_state.allocated_ptrs[i].ptr == ptr) {
            *size = mem_state.allocated_ptrs[i].size;
            for (u32 j = i + 1; j < mem_state.index; ++i, ++j)
                mem_state.allocated_ptrs[i] = mem_state.allocated_ptrs[j];
            --mem_state.index;
            return true;
        }
    }

    // Might Reach here if "Memory allocation is not tracked" error is
    // displayed
    return false;
}

/**
 * @brief (INTERNAL FUNCTION) Updates the state.
 *
 * @param size Allocated size if allocation, else will be ignored
 * @param ptr Allocated pointer
 * @param is_allocation true if allocation, false if deallocation
 *
 * @return true on success, else fasle.
 */
b8 updatedMemoryState(u64 size, void *ptr, b8 is_allocation) {
    if (!mem_state.initialized) {
        // TODO: Write assertion
        SERROR("updateMemoryState called before initializing the memory");
        return false;
    }

    if (!updateAllocatedPtrs(ptr, &size, is_allocation)) {
        return false;
    }

    if (is_allocation) mem_state.total_allocated += size;
    else mem_state.total_allocated -= size;

    return true;
}

/**
 * @brief Similar to malloc.
 *
 * @param size Size of bytes to allocate
 *
 * @return Pointer to allocated memory on success else NULL.
 */
void *smalloc(u64 size) {
    // TODO: replace with assertion
    if (!mem_state.initialized) {
        SERROR("smalloc called before initializing the memory");
        return NULL;
    }

    void *ptr = platformAllocateMemory(size);

    if (ptr && !updatedMemoryState(size, ptr, true)) {
        SERROR("Memory allocation is not being tracked");
    }

    return ptr;
}

/**
 * @brief Similar to calloc.
 *
 * @param nmemb Number of blocks
 * @param size Size of each block
 *
 * @return Pointer to allocated memory on success else NULL.
 */
void *scalloc(u64 nmemb, u64 size) {
    // TODO: replace with assertion
    if (!mem_state.initialized) {
        SERROR("scalloc called before initializing the memory");
        return NULL;
    }

    u64 total_size = size * nmemb;

    void *ptr = platformAllocateMemory(total_size);

    if (ptr && !updatedMemoryState(total_size, ptr, true)) {
        SERROR("Failed to track the Memory allocation");
    }

    platformZeroOutMemory(ptr, total_size);

    return ptr;
}

/**
 * @brief Similar to realloc.
 *
 * If size is zero calls sfree(returns null) and if ptr is NULL calls smalloc.
 * If ptr is NULL as well as size is zero throws error If ptr is NULL as well as
 * size is zero shows warning and returns NULL.
 *
 * @param ptr Pointer to the Allocated Memory
 * @param size New size
 *
 * @return NULL if failed else pointer to the Memory allocated with new size.
 */
void *srealloc(void *ptr, u64 size) {
    // TODO: replace with assertion
    if (!mem_state.initialized) {
        SERROR("srealloc called before initializing the memory");
        return NULL;
    }

    if (!ptr && !size) {
        SWARN("srealloc called with NULL pointer and 0 size");
        return NULL;
    }
    if (!size) {
        sfree(ptr);
        return NULL;
    }

    if (!ptr) return smalloc(size);

    void *p = platformReallocateMemory(ptr, size);
    if (p) {
        if (!updatedMemoryState(0, ptr, false))
            SINFO("Untracked Memory allocation is being reallocated, trying to "
                  "retrack...");

        if (!updatedMemoryState(size, p, true))
            SERROR("Failed to track the Memory allocation");
    }

    return p;
}

/**
 * @brief Similar to free.
 *
 * @param ptr Pointer to the memory to be deallocated
 */
void sfree(void *ptr) {
    // TODO: replace with assertion
    if (!mem_state.initialized) {
        SERROR("sfree called before initializing the memory");
        return;
    }

    if (!updatedMemoryState(0, ptr, false))
        SINFO("Untracked Memory allocation is being deallocated");

    platformDeallocateMemory(ptr);
}
