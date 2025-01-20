#pragma once

#include "arenaalloc.h"
#include "defines.h"

b8 initializeMemory(void);

void shutdownMemory(void);

SAPI void *sMalloc(u64 size);

SAPI void *sCalloc(u64 nmemb, u64 size);

SAPI void *sRealloc(void *ptr, u64 size);

SAPI void sFree(void *ptr);

SAPI void sMemLogState(void);

SAPI void *sMemZeroOut(void *ptr, u64 size);

SAPI void *sMemSet(void *ptr, u64 size, u8 value);

/**
 * @brief Zero out the memory.
 *
 * @param ptr Pointer to the memory
 * @param size Size of memory to be zeroed out
 *
 * @return Returns the given pointer.
 */
#define sMemZeroOut(ptr, size) sMemSet(ptr, size, 0)

SAPI void *sMemCopy(void *dest, const void *src, u64 size);

SAPI void *sMemMove(void *dest, void *src, u64 size);
