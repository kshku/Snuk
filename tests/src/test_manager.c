#include "test_manager.h"

#include <core/logger.h>
#include <ds/darray.h>

Test *initializeTestManager(void) {
    Test *tests = darrayCreateWithSize(Test, 1);
    return tests;
}

void shutdownTestManager(Test *tests) {
    darrayDestroy(tests);
}

Test *testManagerRegister(Test *tests, pfn_test f, const char *desc) {
    darrayPush(tests, ((Test){.f = f, .desc = desc}));
    return tests;
}

void testManagerRun(Test *tests) {
    // TODO: Log time taken to run tests
    u16 passed, failed, skipped;
    passed = failed = skipped = 0;

    u64 length = darrayLength(tests);

    for (u32 i = 0; i < length; ++i) {
        switch (tests[i].f()) {
            case FAIL:
                ++failed;
                sError("[FAILED]: %s", tests[i].desc);
                break;
            case PASS:
                ++passed;
                break;
            case BYPASS:
                ++skipped;
                sWarn("[SKIPPED]: %s", tests[i].desc);
                break;
            default:
                sError("Unkown result from the test with description '%s'",
                       tests[i].desc);
                break;
        }
        sInfo("Executed %d of %d (skipped %d) %d %s", i + 1, length, skipped,
              (failed ? failed : passed), (failed ? "FAILED" : "PASSED"));
    }
    sInfo("Results: %d passed, %d failed, %d skipped.", passed, failed,
          skipped);
}
