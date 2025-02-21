#pragma once

#include "defines.h"

void *platformAllocateMemory(u64 size);

void platformDeallocateMemory(void *ptr, u64 size);

i64 platformGetPageSize(void);
