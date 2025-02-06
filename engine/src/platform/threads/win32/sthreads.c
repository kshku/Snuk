#include "platform/sthreads.h"

#if defined(SPLATFORM_THREADS_WINDOWS)

/**
 * @brief Windows thread calling function wrapper.
 *
 * @param wrap The array containing actual function as well as the arg
 *
 * @return The return value of function casted to DWORD.
 */
static DWORD sThreadWrapper(void wrap[2]) {
    sThread_func func = ((sThread_func *)wrap)[0];
    void *data = ((void **)wrap)[1];
    return (DWORD)(uptr)func(data);
}

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
    void *wrap[2] = {(void *)func, data};
    *thread = CreateThread(NULL, 0, sThreadWrapper, wrap, 0, NULL);
    return thread ? true : false;
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
    if (ret) *ret = NULL;

    if (!WaitForSingleObject(thread, INFINITE)) return false;

    if (ret) {
        DWORD exitcode;
        if (GetExitCodeThread(thread, &exitcode))
            *ret = (void *)(utpr)(exitcode);
    }

    CloseHandle(thread);

    thread = NULL;

    return true;
}

/**
 * @brief Exit with exitcode.
 *
 * Terminates the calling thread.
 *
 * @param exitcode The exit code
 */
void sThreadExit(void *ret) {
    ExitThread((DWORD)(uptr)ret);
}

/**
 * @brief Get the current thread.
 *
 * @return Current sThread
 */
sThread sThreadCurrent(void) {
    return GetCurrentThread();
}

/**
 * @brief Sleep for given milli seconds.
 *
 * @param ms The time in milli seconds
 */
void sThreadSleep(u64 ms) {
    // TODO:
    if (ms == 0 || ms == INFINITE) {
        ms = 1;
    }

    Sleep(ms);
}

/**
 * @brief Yield the timeslice to other threads.
 */
void sThreadYield(void) {
    Sleep(0);
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
    // TODO: Do something similar to the pthread_cancel instead of forcefull
    // termination
    return TerminateThread(thread, exitcode);
}

#endif
