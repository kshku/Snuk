#include "arenaalloc.h"

#include "memory.h"

/**
 * @brief Create arena allocator with the given size of memory.
 *
 * @note The size to allocate should be written in arena->size field.
 *
 * @param arena Pointer to the arena
 *
 * @return true if succeeds, false otherwise.
 */
b8 sArenaCreate(SArena *restrict arena) {
    uptr memory = (uptr)sMalloc(arena->size);
    if (!memory) return false;
    arena->base = arena->head = memory;
    return true;
}

/**
 * @brief Allocate given size of memory from arena.
 *
 * @param arena Pointer to the arena
 * @param size Size to allocate
 *
 * @return If arena was not allocated or the size requested is greater than size
 * available returns NULL, else poitner to allocated memory.
 */
void *sArenaAlloc(SArena *restrict arena, u64 size) {
    if (!arena->base) return NULL;

    if (arena->head + size <= arena->base + arena->size) {
        void *ptr = (void *)arena->head;
        arena->head += size;
        return ptr;
    }

    return NULL;
}

/**
 * @brief Clear all the allocations from the arena.
 */
void sArenaClear(SArena *restrict arena) {
    arena->head = arena->base;
}

/**
 * @brief Destory the arena.
 *
 * Deallocates the memory allocated for the arena.
 *
 * @param arena The pointer to arena
 */
void sArenaDestroy(SArena *restrict arena) {
    if (arena->base) sFree((void *)arena->base);
}
