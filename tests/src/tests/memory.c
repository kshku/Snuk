#include "memory.h"

#include <core/memory/memory.h>

u8 auto_deallocation(void) {
    u64 *ptr = (u64 *)sMalloc(10 * sizeof(u64));
    ptr = (u64 *)sCalloc(10, sizeof(u64));
    UNUSED(ptr);
    return PASS;
}

u8 realloc_test(void) {
    u64 *ptr = (u64 *)sRealloc(NULL, 0);
    if (ptr) return FAIL;
    ptr = (u64 *)sRealloc(NULL, 8);
    if (!ptr) return FAIL;
    ptr = (u64 *)sRealloc(ptr, 0);
    return PASS;
}

Test *core_memory_register_tests(Test *tests) {
    tests = testManagerRegister(tests, auto_deallocation, "auto deallocation");
    tests = testManagerRegister(tests, realloc_test, "realloc test");
    return tests;
}
