#include "memory.h"

#include <core/memory.h>

#include "../test_manager.h"

u8 auto_deallocation() {
    u64 *ptr = (u64 *)smalloc(10 * sizeof(u64));
    ptr = (u64 *)scalloc(10, sizeof(u64));
    return PASS;
}

u8 realloc_test() {
    u64 *ptr = (u64 *)srealloc(NULL, 0);
    if (ptr) return FAIL;
    ptr = (u64 *)srealloc(NULL, 8);
    if (!ptr) return FAIL;
    ptr = (u64 *)srealloc(ptr, 0);
    return PASS;
}

void core_memory_register_tests() {
    testManagerRegister(auto_deallocation, "auto deallocation");
    testManagerRegister(realloc_test, "realloc test");
}
