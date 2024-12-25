#include "../../memory.h"

#ifdef SPLATFORM_OS_LINUX

    #include <stdlib.h>
    #include <string.h>

/**
 * @brief Memory allocater implementation for Linux.
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
 * @brief Memory deallocater implementation for Linux.
 *
 * @param ptr Pointer to the allocated memory
 */
void platformDeallocateMemory(void *ptr) {
    // TODO: platform specific calls instead of library calls
    free(ptr);
}

/**
 * @brief Memory reallocater implementation for Linux.
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
 * @brief Zero out memory implementation for Linux.
 *
 * @param ptr Pointer to the memory to zero out
 * @param size Size of the memory to be zeroed out
 *
 * @return Returns the given pointer.
 */
void *platformZeroOutMemory(void *ptr, u64 size) {
    // TODO: Don't use library call
    return memset(ptr, 0, size);
}

/**
 * @brief Mem set implementation for Linux.
 *
 * @param ptr Pointer to the memory
 * @param size Size of the memory
 * @param value value to which each byte to be set
 *
 * @return Returns the given pointer.
 */
void *platformMemSet(void *ptr, u64 size, u8 value) {
    return memset(ptr, value, size);
}

/**
 * @brief Copy memory from source to destination.
 *
 * @param dest Destination pointer
 * @param src Source pointer
 * @param size Number of bytes to copy
 *
 * @return Returns the destination pointer.

 * @note Source and destination should not overlap.
 */
void *platformMemCopy(void *dest, void *src, u64 size) {
    // TODO: Don't use library call
    return memcpy(dest, src, size);
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
void *platformMemMove(void *dest, void *src, u64 size) {
    // TODO: Don't use library call
    return memmove(dest, src, size);
}

#endif
