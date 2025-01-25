#include "memory.h"

#include <core/memory/memory.h>

u8 realloc_test(void) {
    // Should print error
    u64 *ptr = (u64 *)sRealloc(NULL, 0);
    if (ptr) return FAIL;

    // Should allocate 8bytes
    ptr = (u64 *)sRealloc(NULL, 8);
    if (!ptr) return FAIL;

    // Should do nothing
    if (ptr != sRealloc(ptr, 8)) return FAIL;

    // Should release 4 bytes
    if (ptr != sRealloc(ptr, 4)) return FAIL;

    // Should use the released 4 bytes
    void *new_ptr = sMalloc(4);
    if ((u8 *)new_ptr != (u8 *)ptr + 4 + 8) return FAIL;

    // Should allocate new memory
    ptr = sRealloc(ptr, 8);
    if ((u8 *)ptr == (u8 *)new_ptr - 4 - 8) return FAIL;

    sFree(new_ptr);

    if ((u8 *)new_ptr - 4 - 8 != (u8 *)sMalloc(8)) return FAIL;

    sFree((u8 *)new_ptr - 4 - 8);

    ptr = (u64 *)sRealloc(ptr, 0);
    return PASS;
}

Test *core_memory_register_tests(Test *tests) {
    tests = testManagerRegister(tests, realloc_test, "realloc test");
    return tests;
}
