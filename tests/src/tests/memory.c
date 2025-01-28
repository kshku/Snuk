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

    // Should not release 4 bytes
    if (ptr != sRealloc(ptr, 4)) return FAIL;
    if (*((u64 *)ptr - 1) != 16) return FAIL;

    sRealloc(ptr, 0);

    // Should use released memory
    void *new_ptr = sMalloc(4);
    if (new_ptr != ptr) return FAIL;

    sFree(new_ptr);

    return PASS;
}

u8 allocate_pages(void) {
    void *ptr = sMemAllocatePages(1);
    if (!ptr) return FAIL;
    sMemLogState();
    sMemDeallocatePages(ptr, 1);
    sMemLogState();

    return PASS;
}

Test *core_memory_register_tests(Test *tests) {
    tests = testManagerRegister(tests, realloc_test, "realloc test");
    tests = testManagerRegister(tests, allocate_pages, "allocate pages");
    return tests;
}
