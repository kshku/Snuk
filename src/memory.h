#pragma once

#include "defines.h"

#include <snmemory/snmemory.h>

#define KIB(x) ((x) * 1024)
#define MIB(x) (KIB((x) * 1024))
#define GIB(x) (MIB((x) * 1024))
/**
 * @brief Retrieves the system page size.
 *
 * This macro wraps the underlying sn_vm_get_page_size() function.
 *
 * @return System page size.
 */
#define snuk_page_size() sn_vm_get_page_size()
/**
 * @brief Initializes the memory system.
 *
 * Reserves a block of memory of the specified size for use by the system.
 *
 * @param reserve_size Size of memory to reserve in bytes.
 * @return true if initialization was successful, false otherwise.
 */
bool snuk_memory_init(uint64_t reserve_size);
/**
 * @brief Deinitializes the memory system.
 *
 * Releases any resources or memory previously reserved by the memory system.
 */
void snuk_memory_deinit(void);
/**
 * @brief Allocates a number of memory pages.
 *
 * Allocates a contiguous block of memory consisting of the specified
 * number of pages.
 * should be freed in stack order, first allocated should be freed last
 * @param pages Number of memory pages to allocate.
 * @return Pointer to the allocated memory pages, or NULL if allocation fails.
 */
void *snuk_allocate_pages(uint32_t pages);
/**
 * @brief Frees a block of allocated memory pages.
 *
 * Releases a contiguous block of memory pages starting from the given base pointer.
 *
 * @param base Pointer to the beginning of the memory block to be freed.
 * @param pages Number of memory pages to free.
 */
void snuk_free_pages(void *base, uint32_t pages);
/**
 * @brief Allocates a block of memory with a specified alignment.
 *
 * Allocates a block of memory of the given size and ensures that the
 * returned pointer is aligned to the specified boundary.
 *
 * @param size Number of bytes to allocate.
 * @param align Alignment requirement for the allocated memory.
 * @return Pointer to the allocated memory, or NULL if allocation fails.
 */
void *snuk_alloc(uint64_t size, uint64_t align);
/**
 * @brief Resizes a previously allocated memory block.
 *
 * Reallocates the given memory block to a new size with the specified alignment.
 * The contents of the original memory are preserved up to the minimum of the old
 * and new sizes.
 *
 * @param ptr Pointer to the previously allocated memory block.
 * @param new_size New size of the memory block in bytes.
 * @param align Alignment requirement for the reallocated memory.
 * @return Pointer to the reallocated memory, or NULL if reallocation fails.
 */
void *snuk_realloc(void *ptr, uint64_t new_size, uint64_t align);
/**
 * @brief Frees a previously allocated block of memory.
 *
 * Releases the memory pointed to by the given pointer.
 *
 * @param ptr Pointer to the memory block to be freed.
 */
void snuk_free(void *ptr);