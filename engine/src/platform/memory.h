#pragma once

#include "defines.h"

void *platformAllocateMemory(u64 size);

void platformDeallocateMemory(void *ptr);

void *platformReallocateMemory(void *ptr, u64 size);

void *platformZeroOutMemory(void *ptr, u64 size);

void *platformMemSet(void *ptr, u64 size, u8 value);

void *platformMemCopy(void *dest, void *src, u64 size);

void *platformMemMove(void *dest, void *src, u64 size);
