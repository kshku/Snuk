#include "../memory.h"

#ifdef SPLATFORM_LINUX

    #include <stdlib.h>
    #include <string.h>

/**
 * @brief Memory allocater for Linux.
 *
 * @param size Size in bytes to be allocated
 *
 * @return On success pointer to allocated memory, else NULL.
 */
void *platformAllocateMemory(u64 size) {
    // TODO: platform specific calls instead of library calls
    return malloc(size);
}

/**
 * @brief Memory deallocater for Linux.
 *
 * @param ptr Pointer to the allocated memory
 */
void platformDeallocateMemory(void *ptr) {
    // TODO: platform specific calls instead of library calls
    free(ptr);
}

/**
 * @brief Memory reallocater for Linux.
 *
 * @param ptr Pointer to the allocated memory
 * @param size New size
 *
 * @return Pointer to the reallocated memory.
 */
void *platformReallocateMemory(void *ptr, u64 size) {
    // TODO: platform specific calls instead of library calls
    return realloc(ptr, size);
}

/**
 * @brief Zero out memory for Linux.
 *
 * @param ptr Pointer to the memory to zero out
 * @param size Size of the memory to be zeroed out
 *
 * @return Returns the passed ptr after setting it to zero.
 */
void *platformZeroOutMemory(void *ptr, u64 size) {
    // TODO: Don't use library call
    memset(ptr, 0, size);
    return ptr;
}

#endif
