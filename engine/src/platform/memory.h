#pragma once

#include "defines.h"

void *platformAllocateMemory(u64 size);

void platformDeallocateMemory(void *ptr, u64 size);

void *platformReallocateMemory(void *ptr, u64 new_size, u64 old_size);

i64 platformGetPageSize(void);

void *platformZeroOutMemory(void *ptr, u64 size);

void *platformMemSet(void *ptr, u64 size, u8 value);

void *platformMemCopy(void *dest, void *src, u64 size);

void *platformMemMove(void *dest, void *src, u64 size);
