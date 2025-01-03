#pragma once

#include "defines.h"

b8 initializeMemory(void);

void shutdownMemory(void);

SAPI void *sMalloc(u64 size);

SAPI void *sCalloc(u64 nmemb, u64 size);

SAPI void *sRealloc(void *ptr, u64 size);

SAPI void sFree(void *ptr);

SAPI void sLogMemState(void);

SAPI void *sMemZeroOut(void *ptr, u64 size);

SAPI void *sMemSet(void *ptr, u64 size, u8 value);

SAPI void *sMemCopy(void *dest, void *src, u64 size);

SAPI void *sMemMove(void *dest, void *src, u64 size);

// SAPI void *sMemBlockSet(void *ptr, u64 size);
