#include "platform/sthreads.h"

#ifdef SPLATFORM_THREADS_PTHREAD

    #include <errno.h>
    #include <unistd.h>

/**
 * @brief Create and start a new thread.
 *
 * @param thread Pointer to the sThread
 * @param func The function that thread runs
 * @param data The data passed as paramter to the function
 *
 * @return Returns true on success, else false.
 */
b8 sThreadCreate(sThread *thread, sThread_func func, void *data) {
    return !pthread_create(thread, NULL, func, data);
}

/**
 * @brief Wait till the given thread completes.
 *
 * @param thread The thread to join
 * @param ret The value returned by the thread (can be NULL)
 *
 * @return Returns the exit code (return value) of thread.
 */
b8 sThreadJoin(sThread thread, void **ret) {
    return !pthread_join(thread, ret);
}

/**
 * @brief Exit with exitcode.
 *
 * Terminates the calling thread.
 *
 * @param exitcode The exit code
 */
void sThreadExit(void *ret) {
    pthread_exit(ret);
}

/**
 * @brief Get the current thread.
 *
 * @return Current sThread
 */
sThread sThreadCurrent(void) {
    return pthread_self();
}

/**
 * @brief Sleep for given milli seconds.
 *
 * @param ms The time in milli seconds
 */
void sThreadSleep(u64 ms) {
    // TODO:
    switch (usleep(ms * 1000)) {
        case 0:
            break;
        case EINTR:
        case EINVAL:
            break;
        default:
            break;
    }
}

/**
 * @brief Yield the timeslice to other threads.
 */
void sThreadYield(void) {
    sched_yield();
}

/**
 * @brief Terminate the given thread with given exit code.
 *
 * @param thread The thread to be terminated
 * @param exitcode The exit code for the terminating thread
 *
 * @return Returns true if thread is terminated else false.
 */
b8 sThreadTerminate(sThread thread, u64 exitcode) {
    // // TODO: Do something similar to the pthread_cancel instead of forcefull
    // // termination
    // return TerminateThread(thread, exitcode);
}

#endif
