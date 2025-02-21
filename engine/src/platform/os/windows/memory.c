#include "platform/memory.h"

#ifdef SPLATFORM_OS_WINDOWS

    #include <Windows.h>
    #include <string.h>

/**
 * @brief Memory allocater implementation for Windows.
 *
 * @param size Size in bytes to be allocated
 *
 * @return On success pointer to allocated memory, else NULL.
 */
void *platformAllocateMemory(u64 size) {
    return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

/**
 * @brief Memory deallocater implementation for Windows.
 *
 * @param ptr Pointer to the allocated memory
 * @param size Size of the allocated memory
 */
void platformDeallocateMemory(void *ptr, u64 size) {
    UNUSED(size);
    VirtualFree(ptr, 0, MEM_RELEASE);
}

/**
 * @brief Get the page size implementation for Windows.
 *
 * @return Page size.
 */
i64 platformGetPageSize(void) {
    static SYSTEM_INFO system_info;

    if (!system_info.dwPageSize) GetSystemInfo(&system_info);

    return system_info.dwPageSize;
}

#endif
