#pragma once

#include "defines.h"

#define FAIL 0
#define PASS 1
#define BYPASS 2

typedef u8 (*pfn_test)(void);

void initializeTestManager();

void shutdownTestManager();

void testManagerRegister(pfn_test f, const char *desc);

void testManagerRun();
