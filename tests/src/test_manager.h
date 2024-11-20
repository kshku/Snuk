#pragma once

#include "defines.h"

#define FAIL 0
#define PASS 1
#define BYPASS 2

typedef u8 (*pfn_test)(void);

typedef struct Test {
        pfn_test f;
        const char *desc;
} Test;

Test *initializeTestManager(void);

void shutdownTestManager(Test *tests);

Test *testManagerRegister(Test *tests, pfn_test f, const char *desc);

void testManagerRun(Test *test);
