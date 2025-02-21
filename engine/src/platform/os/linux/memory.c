#include "platform/memory.h"

#ifdef SPLATFORM_OS_LINUX

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
 * @brief Get the page size implementation for Linux.
 *
 * @return Page size.
 */
i64 platformGetPageSize(void) {
    return sysconf(_SC_PAGE_SIZE);
}

#endif
