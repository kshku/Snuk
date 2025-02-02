#include "atomic.h"

#include <core/assertions.h>
#include <platform/atomic.h>

u8 atomic_test(void) {
    DEBUG_BREAK;
    satomic_i32 ai = SATOMIC_CREATE(0);
    SATOMIC_STORE(&ai, 1000);

    if (SATOMIC_LOAD(&ai) != 1000) return FAIL;

    if (SATOMIC_EXCHANGE(&ai, 100) != 1000) return FAIL;

    i32 expect = 100;
    if (!SATOMIC_COMPARE_EXCHANGE(&ai, &expect, 1000)) return FAIL;

    if (SATOMIC_COMPARE_EXCHANGE(&ai, &expect, 1000)) return FAIL;

    return PASS;
}

Test *atomic_tests(Test *tests) {
    testManagerRegister(tests, atomic_test, "atomic tests");
    return tests;
}
