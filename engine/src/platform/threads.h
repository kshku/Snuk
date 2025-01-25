#pragma once

#include "defines.h"

#if defined(SPLATFORM_OS_LINUX)
    #define SPLATFORM_THREADS_PTHREAD
#elif defined(SPLATFORM_OS_WINDOWS)
    #define SPLATFORM_THREADS_WINDOWS
#endif

void platformThreadCreate();

void platformThreadTerminate();

void platformThreadRequestToExit();

void platformThreadSleep();

void platformThreadYield();

void platformThreadJoin();
