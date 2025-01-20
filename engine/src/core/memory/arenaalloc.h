#pragma once

#include "defines.h"

typedef struct SArena {
        u64 size;
        u8 *base;
        u8 *head;
} SArena;

SAPI b8 sArenaCreate(SArena *arena);

SAPI void *sArenaAlloc(SArena *arena, u64 size);

SAPI void sArenaClear(SArena *arena);

SAPI void sArenaDestroy(SArena *arena);
