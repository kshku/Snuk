#include "../../memory.h"

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
 * @brief Memory reallocater implementation for Windows.
 *
 * @param ptr Pointer to the allocated memory
 * @param new_size New size
 * @param old_size Old size (Current size)
 *
 * @return Pointer to the reallocated memory.
 */
void *platformReallocateMemory(void *ptr, u64 new_size, u64 old_size) {
    void *p =
        VirtualAlloc(NULL, new_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (p) {
        u64 length = MIN(new_size, old_size);
        u8 *pc = (u8 *)p;
        u8 *pc_old = (u8 *)ptr;
        for (u64 i = 0; i < length; ++i, ++pc, ++pc_old) *pc = *pc_old;
    }

    return p;
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

/**
 * @brief Zero out memory implementation for Windows.
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
 * @brief Mem set implementation for Windows.
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
