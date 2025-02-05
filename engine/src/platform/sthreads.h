#pragma once

#include <threads.h>

#include "defines.h"

#if defined(SPLATFORM_OS_LINUX)
    #define SPLATFORM_THREADS_PTHREAD
    #undef SPLATFORM_THREADS_PTHREAD
#elif defined(SPLATFORM_OS_WINDOWS)
    #define SPLATFORM_THREADS_WINDOWS
    #undef SPLATFORM_THREADS_WINDOWS
#endif

#ifdef __STDC_NO_THREADS__
    #error "No support for threads"
#endif

typedef struct sthread {
        thrd_t id;
} sthread;

typedef struct stime {
        u64 seconds;
        u64 nanoseconds;
} stime;

typedef i32 (*thread_func_t)(void *);

SAPI b8 sThreadCreate(sthread *thread, thread_func_t func, void *arg);

SAPI b8 sThreadEqual(sthread t1, sthread t2);

SAPI sthread sThreadCurrent(void);

SAPI void sThreadSleep(stime time);

SAPI void sThreadYield(void);

SAPI void sThreadExit(i32 ret);

SAPI void sThreadDetach(sthread thread);

SAPI i32 sThreadJoin(sthread thread);
