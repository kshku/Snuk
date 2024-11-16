#pragma once

#include "defines.h"

b8 initializeMemory();

void shutdownMemory();

SAPI void *smalloc(u64 size);

SAPI void *scalloc(u64 nmemb, u64 size);

SAPI void *srealloc(void *ptr, u64 size);

SAPI void sfree(void *ptr);
