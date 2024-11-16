#pragma once

#include "defines.h"

void *platformAllocateMemory(u64 size);

void platformDeallocateMemory(void *ptr);

void *platformReallocateMemory(void *ptr, u64 size);

void *platformZeroOutMemory(void *ptr, u64 size);
