#include "../../memory.h"

#ifdef SPLATFORM_OS_LINUX

    #define _GNU_SOURCE
    #include <string.h>
    #include <sys/mman.h>
    #include <unistd.h>

    #include "core/logger.h"

/**
 * @brief Memory allocater implementation for Linux.
 *
 * @param size Size in bytes to be allocated
 *
 * @return On success pointer to allocated memory, else NULL.
 */
void *platformAllocateMemory(u64 size) {
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE,
                     MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (ptr == MAP_FAILED) sDebug("Failed to mmap");
    return ptr == MAP_FAILED ? NULL : ptr;
}

/**
 * @brief Memory deallocater implementation for Linux.
 *
 * @param ptr Pointer to the allocated memory
 * @param size The size of allocation
 */
void platformDeallocateMemory(void *ptr, u64 size) {
    if (munmap(ptr, size) == -1) sDebug("Failed to munmap");
}

/**
 * @brief Memory reallocater implementation for Linux.
 *
 * @param ptr Pointer to the allocated memory
 * @param new_size New size
 * @param old_size Old size (Current size)
 *
 * @return Pointer to the reallocated memory.
 */
void *platformReallocateMemory(void *ptr, u64 new_size, u64 old_size) {
    void *p = mremap(ptr, old_size, new_size, MREMAP_MAYMOVE);
    if (p == MAP_FAILED) return NULL;
    return p;
}

/**
 * @brief Get the page size implementation for Linux.
 *
 * @return Page size.
 */
i64 platformGetPageSize(void) {
    return sysconf(_SC_PAGE_SIZE);
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
