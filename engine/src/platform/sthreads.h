#pragma once

#include "defines.h"

#if defined(SPLATFORM_OS_LINUX)
    #define SPLATFORM_THREADS_PTHREAD
#elif defined(SPLATFORM_OS_WINDOWS)
    #define SPLATFORM_THREADS_WINDOWS
#endif

typedef struct sThread {
        void *handle;
} sThread;

typedef u64 (*sThread_func)(void *data);

SAPI u64 sThreadSize(void);

SAPI b8 sThreadCreate(sThread *thread, sThread_func func, void *data);

SAPI u64 sThreadJoin(sThread thread);

SAPI void sThreadExit(u64 exitcode);

SAPI sThread sThreadCurrent(void);

SAPI void sThreadSleep(u64 ms);

SAPI void sThreadYield(void);

SAPI b8 sThreadTerminate(sThread thread, u64 exitcode);
