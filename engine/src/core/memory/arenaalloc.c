#include "arenaalloc.h"

#include "memory.h"

// TODO: For error handling
// ? Should Arena allocator ask for size or just dynamically allocate the
// memory?

b8 sArenaCreate(SArena *arena) {
    void *memory = sMalloc(arena->size);
    if (!memory) return false;
    arena->base = arena->head = (u8 *)memory;
    return true;
}

void *sArenaAlloc(SArena *arena, u64 size) {
    if (arena->head + size <= arena->base + arena->size) {
        void *ptr = arena->head;
        arena->head += size;
        return ptr;
    }

    return NULL;
}

void sArenaClear(SArena *arena) {
    arena->head = (u8 *)arena->base;
}

void sArenaDestroy(SArena *arena) {
    sFree(arena->base);
}
