#pragma once

#include "defines.h"

#if defined(SPLATFORM_OS_LINUX)
    #define SPLATFORM_THREADS_PTHREAD
    #include <pthread.h>
typedef pthread_t sThread;
#elif defined(SPLATFORM_OS_WINDOWS)
    #define SPLATFORM_THREADS_WINDOWS
    #include <Windows.h>
typedef HANDLE sThread;
#endif

typedef void *(*sThread_func)(void *data);

SAPI b8 sThreadCreate(sThread *thread, sThread_func func, void *data);

SAPI b8 sThreadJoin(sThread thread, void **ret);

SAPI void sThreadExit(void *ret);

SAPI sThread sThreadCurrent(void);

SAPI void sThreadSleep(u64 ms);

SAPI void sThreadYield(void);

// SAPI b8 sThreadTerminate(sThread thread, u64 exitcode);
