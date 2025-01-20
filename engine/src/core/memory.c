#include "memory.h"

#include "assertions.h"
#include "core/logger.h"
#include "platform/memory.h"

#define KiB 1024
#define MiB (1024 * KiB)
#define GiB (1024 * MiB)

#define BUFFER_SIZE (512 * KiB)

// TODO: Write custom allocators for different situations

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

        u8 *buffer;
        u8 *free_buffer_start;
        u64 free_buffer_size;
} MemState;

static MemState mem_state;

/**
 * @brief Reallocate the memory [INTERNAL FUNCTION].
 *
 * This function simulates the realloc's behaviour by allocating space for new
 * size of memory, copying and deallocatin old memory.
 *
 * @param ptr Pointer
 * @param new_size The new size
 * @param prev_size Previous size or current size
 *
 * @return NULL on failure and data is not modified, if successful, returns
 * pointer with new size.
 */
static void *reallocate(void *ptr, u64 new_size, u64 prev_size) {
    void *p = platformAllocateMemory(new_size);

    if (p) {
        sMemCopy(p, ptr, MIN(new_size, prev_size));
        platformDeallocateMemory(ptr, prev_size);
    }

    return p;
}

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

    sMemZeroOut(&mem_state, sizeof(MemState));

    mem_state.total_allocated = 0;
    mem_state.index = 0;
    mem_state.size = 1;

    mem_state.allocated_ptrs = (PtrSizePair *)platformAllocateMemory(
        mem_state.size * sizeof(PtrSizePair));

    if (!mem_state.allocated_ptrs) return false;

    mem_state.free_buffer_size = BUFFER_SIZE;
    mem_state.buffer = (u8 *)platformAllocateMemory(BUFFER_SIZE);
    if (!mem_state.buffer) {
        platformDeallocateMemory(mem_state.allocated_ptrs, mem_state.size);
        return false;
    }
    mem_state.free_buffer_start = mem_state.buffer;

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
    sMemLogState();

    if (mem_state.allocated_ptrs) {
        for (u32 i = 0; i < mem_state.index; ++i) {
            sTrace("Deallocating %ld bytes of memory",
                   mem_state.allocated_ptrs[i].size);
            mem_state.total_allocated -= mem_state.allocated_ptrs[i].size;
            // platformDeallocateMemory(mem_state.allocated_ptrs[i].ptr);
        }

        platformDeallocateMemory(mem_state.allocated_ptrs, mem_state.size);
        mem_state.allocated_ptrs = NULL;
    }

    sTrace("After deallocation");
    sMemLogState();

    platformDeallocateMemory(mem_state.buffer, BUFFER_SIZE);
    mem_state.free_buffer_start = NULL;

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
            void *p = reallocate(mem_state.allocated_ptrs,
                                 mem_state.size * sizeof(PtrSizePair),
                                 (mem_state.size - 2) * sizeof(PtrSizePair));
            // void *p = platformReallocateMemory(
            //     mem_state.allocated_ptrs,
            //     (mem_state.size * sizeof(PtrSizePair)));
            if (!p) {
                sError("updateAllocatedPtrs reallocation failed");
                mem_state.size -= 2;
                return false;
            }
            mem_state.allocated_ptrs = (PtrSizePair *)p;
        }
        mem_state.allocated_ptrs[mem_state.index++] =
            (PtrSizePair){.ptr = ptr, .size = (*size)};
        return true;
    }

    // for (u32 i = 0; i < mem_state.index; ++i) {
    for (u32 i = mem_state.index - 1; i >= 0; --i) {
        if (mem_state.allocated_ptrs[i].ptr == ptr) {
            *size = mem_state.allocated_ptrs[i].size;
            sMemMove((void *)(mem_state.allocated_ptrs + i),
                     (void *)(mem_state.allocated_ptrs + i + 1),
                     ((mem_state.index - i - 1) * sizeof(PtrSizePair)));
            --mem_state.index;
            return true;
        }
    }

    // Will reach here only when reallocating for the allocated_ptrs fails
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
    sassert_msg(mem_state.initialized, "Memory subsystem is not initialized!");

    if (mem_state.free_buffer_size < size) {
        sFatal("Buffer size is not enough to allocate");
        return NULL;
    }

    void *ptr = (void *)mem_state.free_buffer_start;
    // TODO: This is temporary implementation
    mem_state.free_buffer_start += size + KiB;
    mem_state.free_buffer_size -= size + KiB;

    if (!updatedMemoryState(size, ptr, true))
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
    sassert_msg(mem_state.initialized, "Memory subsystem is not initialized!");

    u64 total_size = size * nmemb;

    // void *ptr = platformAllocateMemory(total_size);
    void *ptr = sMalloc(total_size);
    if (ptr) platformZeroOutMemory(ptr, total_size);

    if (ptr && !updatedMemoryState(total_size, ptr, true))
        sError("Failed to track the Memory allocation");

    return ptr;
}

/**
 * @brief Similar to realloc.
 *
 * If size is zero calls sFree(returns null) and if ptr is NULL calls
 * sMalloc. If ptr is NULL as well as size is zero throws error If ptr is
 * NULL as well as size is zero shows warning and returns NULL.
 *
 * @param ptr Pointer to the Allocated Memory
 * @param size New size
 *
 * @return NULL if failed else pointer to the Memory allocated with new
 * size.
 */
void *sRealloc(void *ptr, u64 size) {
    sassert_msg(mem_state.initialized, "Memory subsystem is not initialized!");

    if (!ptr && !size) {
        sError("sRealloc called with NULL pointer and 0 size");
        return ptr;
    }

    if (!size) {
        sFree(ptr);
        return NULL;
    }

    if (!ptr) return sMalloc(size);

    // TODO: This is temporary implementation
    // Have already left 1 KiB space empty for this temporary implementation
    // So just return this pointer and update the state and return the given
    // pointer

    if (!updatedMemoryState(0, ptr, false))
        sInfo("Untracked Memory allocation is being reallocated, "
              "retracking the memory.");

    if (!updatedMemoryState(size, ptr, true))
        sError("Failed to track the Memory allocation");

    return ptr;
}

/**
 * @brief Similar to free.
 *
 * @param ptr Pointer to the memory to be deallocated
 */
void sFree(void *ptr) {
    sassert_msg(mem_state.initialized, "Memory subsystem is not initialized!");

    // TODO: This is temporary implementation
    // b8 found = false;
    // for (u32 i = mem_state.index - 1; i >= 0; --i) {
    //     if (mem_state.allocated_ptrs[i].ptr == ptr) {
    //         PtrSizePair pair = mem_state.allocated_ptrs[i];
    //         // sMemMove(
    //         //     pair.ptr, (void *)((u8 *)pair.ptr + pair.size),
    //         //     (mem_state.free_buffer_start - ((u8 *)pair.ptr +
    //         pair.size)));

    //         // mem_state.free_buffer_size += pair.size + KiB;
    //         // mem_state.free_buffer_start -= pair.size + KiB;

    //         found = true;
    //         break;
    //     }
    // }
    // if (!found) sFatal("Temporary implementation is failing");

    if (!updatedMemoryState(0, ptr, false))
        sInfo("Untracked memory allocation is being deallocated");
}

/**
 * @brief Log how much memory is allocated.
 */
void sMemLogState(void) {
    sassert_msg(mem_state.initialized, "Memory subsystem is not initialized!");

    f32 allocation = 0;
    c8 ext[4] = "XiB";

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
 * @brief Similar to memset sets each byte in memory to given value.
 *
 * @param ptr Pointer to the memory
 * @param size Size of memory
 * @param value The value to which each byte to be set
 *
 * @return Returns the given pointer.
 */
void *sMemSet(void *ptr, u64 size, u8 value) {
    // TODO: Probably should let the compiler to optimize by converting to SIMD
    // which might be faster than this manual large block copying.
    // I think it is better to set up SIMD things and all then come to optimize.

    // u64 v64 = (value << 56) | (value << 48) | (value << 40) | (value << 32)
    //         | (value << 24) | (value << 16) | (value << 8) | (value);

    // // First 64 bits (8 bytes)
    // u64 i = 0;
    // u64 *p64 = (u64 *)ptr;
    // for (; i + 8 < size; i += 8, ++p64) *p64 = v64;

    // // Next is 32 bits (4 bytes)
    // u32 *p32 = (u32 *)p64;
    // u32 v32 = (u32)v64;
    // for (; i + 4 < size; i += 4, ++p32) *p32 = v32;

    // // Next is 16 bits (2 bytes)
    // u16 *p16 = (u16 *)p32;
    // u16 v16 = (u16)v64;
    // for (; i + 2 < size; i += 2, ++p16) *p16 = v16;

    // // Next is 8 bits (1 bytes)
    // u8 *p = (u8 *)p16;
    // for (; i < size; ++i, ++p) *p = value;

    // And done
    // return ptr;

    u8 *p = (u8 *)ptr;
    for (u64 i = 0; i < size; ++i, ++p) *p = value;
    return ptr;
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
void *sMemCopy(void *dest, const void *src, u64 size) {
    if (dest == src) return dest;

    u8 *d = (u8 *)dest;
    u8 *s = (u8 *)src;
    for (u64 i = 0; i < size; ++i, ++d, ++s) *d = *s;

    return dest;
}

/**
 * @brief Copy memory from source to destination.
 *
 * @param dest Destination pointer
 * @param src Source pointer
 * @param size Number of bytes to copy
 *
 * @return Returns the destination pointer.
 *
 * @note Source and destination may overlap.
 */
void *sMemMove(void *dest, void *src, u64 size) {
    if (dest == src) return dest;

    u8 *d = (u8 *)dest;
    u8 *s = (u8 *)src;
    if (dest > src) {
        d += size;
        s += size;
        for (u64 i = size; i > 0; --i, --d, --s) *d = *s;
    } else {
        for (u64 i = 0; i < size; ++i, ++d, ++s) *d = *s;
    }

    return dest;
}
