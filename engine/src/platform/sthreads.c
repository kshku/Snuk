
#include "sthreads.h"

#include "core/logger.h"

// Just for fun

b8 sThreadCreate(sthread *thread, thread_func_t func, void *arg) {
    if (thrd_create(&thread->id, func, arg) != thrd_success) {
        sError("Failed to create thread");
        return false;
    }

    return true;
}

b8 sThreadEqual(sthread t1, sthread t2) {
    return thrd_equal(t1.id, t2.id);
}

sthread sThreadCurrent(void) {
    return (sthread){.id = thrd_current()};
}

void sThreadSleep(stime time) {
    thrd_sleep((&(struct timespec){.tv_sec = time.seconds,
                                   .tv_nsec = time.nanoseconds}),
               NULL);
}

void sThreadYield(void) {
    thrd_yield();
}

void sThreadExit(i32 ret) {
    thrd_exit(ret);
}

void sThreadDetach(sthread thread) {
    thrd_detach(thread.id);
}

i32 sThreadJoin(sthread thread) {
    i32 ret;
    thrd_join(thread.id, &ret);
    return ret;
}
