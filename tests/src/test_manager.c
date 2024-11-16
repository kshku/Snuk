#include "test_manager.h"

#include <core/logger.h>
#include <core/memory.h>

typedef struct Test {
        pfn_test f;
        const char *desc;
} Test;

typedef struct TestManagerState {
        u32 index;
        u64 size;
        Test *tests;
} TestManagerState;

static TestManagerState tm_state;

void initializeTestManager() {
    tm_state.index = 0;
    tm_state.size = 1;
    tm_state.tests = smalloc(tm_state.size * sizeof(Test));
}

void shutdownTestManager() {
    sfree(tm_state.tests);
}

void testManagerRegister(pfn_test f, const char *desc) {
    if (tm_state.size == tm_state.index) {
        tm_state.size += 2;
        void *ptr = srealloc(tm_state.tests, (tm_state.size * sizeof(Test)));
        if (!ptr) {
            tm_state.size -= 2;
            SERROR("Could not srealloc tests");
            return;
        }
        tm_state.tests = (Test *)ptr;
    }

    tm_state.tests[tm_state.index++] = (Test){.f = f, .desc = desc};
}

void testManagerRun() {
    // TODO: Log time taken to run tests
    u16 passed, failed, skipped;
    passed = failed = skipped = 0;

    for (u32 i = 0; i < tm_state.index; ++i) {
        switch (tm_state.tests[i].f()) {
            case FAIL:
                ++failed;
                SERROR("[FAILED]: %s", tm_state.tests[i].desc);
                break;
            case PASS:
                ++passed;
                break;
            case BYPASS:
                ++skipped;
                SWARN("[SKIPPED]: %s", tm_state.tests[i].desc);
                break;
            default:
                SERROR("Unkown result from the test with description '%s'",
                       tm_state.tests[i].desc);
                break;
        }
        SINFO("Executed %d of %d (skipped %d) %d %s", i + 1, tm_state.index,
              skipped, (failed ? failed : passed),
              (failed ? "FAILED" : "PASSED"));
    }
    SINFO("Results: %d passed, %d failed, %d skipped.", passed, failed,
          skipped);
}
