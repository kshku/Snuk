#include "memory.h"

#include "assertions.h"
#include "core/logger.h"
#include "platform/memory.h"

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
b8 initializeMemory(void) {
    if (mem_state.initialized) {
        sError("Memory system is already initialized, but initializeMemory was "
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
void shutdownMemory(void) {
    if (!mem_state.initialized) {
        sError("shutdownMemory called without initializing Memory");
        return;
    }

    sTrace("Deallocating the allocated memroy in shutdown memory");
    sLogMemState();

    if (mem_state.allocated_ptrs) {
        for (u32 i = 0; i < mem_state.index; ++i) {
            sTrace("Deallocating %ld bytes of memory",
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
    sassert_msg(mem_state.initialized,
                "updateAllocatedPtrs called without initializing memory");

    if (is_allocation) {
        if (mem_state.size == mem_state.index) {
            mem_state.size += 2;
            void *ptr = platformReallocateMemory(
                mem_state.allocated_ptrs,
                (mem_state.size * sizeof(PtrSizePair)));
            if (!ptr) {
                sError("updateAllocatedPtrs reallocation failed");
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
    sassert_msg(mem_state.initialized,
                "updateMemoryState called without initializing the memory");

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
void *sMalloc(u64 size) {
    void *ptr = platformAllocateMemory(size);

    if (!mem_state.initialized) {
        sWarn("sMalloc called without initializing the memory, allocation will "
              "not be tracked.");
        return ptr;
    }

    if (ptr && !updatedMemoryState(size, ptr, true))
        sError("Memory allocation is not being tracked");

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
void *sCalloc(u64 nmemb, u64 size) {
    u64 total_size = size * nmemb;

    void *ptr = platformAllocateMemory(total_size);
    if (ptr) platformZeroOutMemory(ptr, total_size);

    if (!mem_state.initialized) {
        sWarn("sCalloc called without initializing the memory, allocation will "
              "not be tracked.");
        return ptr;
    }

    if (ptr && !updatedMemoryState(total_size, ptr, true))
        sError("Failed to track the Memory allocation");

    return ptr;
}

/**
 * @brief Similar to realloc.
 *
 * If size is zero calls sFree(returns null) and if ptr is NULL calls sMalloc.
 * If ptr is NULL as well as size is zero throws error If ptr is NULL as well as
 * size is zero shows warning and returns NULL.
 *
 * @param ptr Pointer to the Allocated Memory
 * @param size New size
 *
 * @return NULL if failed else pointer to the Memory allocated with new size.
 */
void *sRealloc(void *ptr, u64 size) {
    if (!ptr && !size) {
        sError("sRealloc called with NULL pointer and 0 size");
        return ptr;
    }

    if (!size) {
        sFree(ptr);
        return ptr;
    }

    if (!ptr) return sMalloc(size);

    void *p = platformReallocateMemory(ptr, size);

    if (!mem_state.initialized) {
        sWarn("sRealloc called without initializing the memory, allocation "
              "will not be tracked.");
        return p;
    }

    if (p) {
        if (!updatedMemoryState(0, ptr, false))
            sInfo("Untracked Memory allocation is being reallocated, "
                  "retracking the memory.");

        if (!updatedMemoryState(size, p, true))
            sError("Failed to track the Memory allocation");
    }

    return p;
}

/**
 * @brief Similar to free.
 *
 * @param ptr Pointer to the memory to be deallocated
 */
void sFree(void *ptr) {
    if (!mem_state.initialized) {
        sError("sFree called without initializing the memory");
        platformDeallocateMemory(ptr);
        return;
    }

    if (!updatedMemoryState(0, ptr, false))
        sInfo("Untracked memory allocation is being deallocated");

    // NOTE: ptr may point to somewhere else so that it can be passed to again
    // NOTE: to free function
    platformDeallocateMemory(ptr);
}

/**
 * @brief Log how much memory is allocated.
 */
void sLogMemState(void) {
    if (!mem_state.initialized) {
        sInfo("Memory subsystem is not initialized, no tracked memory "
              "allocation");
    }

    const u64 KiB = 1024;
    const u64 MiB = 1024 * KiB;
    const u64 GiB = 1024 * MiB;

    f32 allocation = 0;
    char ext[4] = "XiB";

    if (mem_state.total_allocated >= GiB) {
        ext[0] = 'G';
        allocation = (f32)mem_state.total_allocated / GiB;
    } else if (mem_state.total_allocated >= MiB) {
        ext[0] = 'M';
        allocation = (f32)mem_state.total_allocated / MiB;
    } else if (mem_state.total_allocated >= KiB) {
        ext[0] = 'K';
        allocation = (f32)mem_state.total_allocated / KiB;
    } else {
        ext[0] = 'B';
        ext[1] = 0;
        allocation = (f32)mem_state.total_allocated;
    }

    sInfo("Total of %f %s memory is allocated", allocation, ext);
}

/**
 * @brief Zero out the memory.
 *
 * @param ptr Pointer to the memory
 * @param size Size of memory to be zeroed out
 *
 * @return Returns the given pointer.
 */
void *sZeroOutMem(void *ptr, u64 size) {
    return platformZeroOutMemory(ptr, size);
}

/**
 * @brief Copy memory from source to destination.
 *
 * @param dest Pointer to destination
 * @param src Pointer to source
 * @param size Number of bytes to copy
 *
 * @return Returns the destination pointer.
 *
 * @note Source and destination should not overlap.
 */
void *sMemCopy(void *dest, void *src, u64 size) {
    return platformMemCopy(dest, src, size);
}

/**
 * @brief Copy memory from source to destination.
 *
 * @param dest Destination pointer
 * @param src Source pointer
 * @param size Number of bytes to copy
 *
 * @return Returns the destination pointer.

 * @note Source and destination may overlap.
 */
void *sMemMove(void *dest, void *src, u64 size) {
    return platformMemMove(dest, src, size);
}
