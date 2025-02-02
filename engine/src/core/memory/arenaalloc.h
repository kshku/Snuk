#pragma once

#include "defines.h"

typedef struct SArena {
        u64 size;
        uptr base;
        uptr head;
} SArena;

SAPI b8 sArenaCreate(SArena *restrict arena);

SAPI void *sArenaAlloc(SArena *restrict arena, u64 size);

SAPI void sArenaClear(SArena *restrict arena);

SAPI void sArenaDestroy(SArena *restrict arena);
