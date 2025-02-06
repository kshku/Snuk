#include "atomic.h"

#include <core/assertions.h>
#include <core/logger.h>
#include <platform/atomic.h>
#include <platform/sthreads.h>

u8 atomic_test(void) {
    satomic_i32 ai = SATOMIC_CREATE(0);
    SATOMIC_STORE(&ai, 1000);

    if (SATOMIC_LOAD(&ai) != 1000) return FAIL;

    if (SATOMIC_EXCHANGE(&ai, 100) != 1000) return FAIL;

    i32 expect = 100;
    if (!SATOMIC_COMPARE_EXCHANGE(&ai, &expect, 1000)) return FAIL;

    if (SATOMIC_COMPARE_EXCHANGE(&ai, &expect, 1000)) return FAIL;

    return PASS;
}

satomic_i32 test = SATOMIC_CREATE(0);

// i32 test = 0;

void *print_now(void *data) {
    sDebug("Now the value is %d", SATOMIC_LOAD((satomic_i32 *)data));
    return NULL;
}

void *f(void *data) {
    UNUSED(data);
    for (u32 i = 0; i < 1000; ++i) SATOMIC_FETCH_ADD(&test, 1);

    sThread t;
    sThreadCreate(&t, print_now, &test);

    return NULL;
}

u8 thread_test(void) {
    sThread threads[10];

    for (u32 i = 0; i < 10; ++i) sThreadCreate(&threads[i], f, NULL);

    void *ret;

    for (u32 i = 0; i < 10; ++i) sThreadJoin(threads[i], &ret);

    if (SATOMIC_LOAD(&test) == 10000) return PASS;
    // sDebug("Total is %d", test);
    // if (test == 10000) return PASS;
    return FAIL;
}

Test *atomic_tests(Test *tests) {
    testManagerRegister(tests, atomic_test, "atomic tests");
    testManagerRegister(tests, thread_test, "thread tests");
    return tests;
}
