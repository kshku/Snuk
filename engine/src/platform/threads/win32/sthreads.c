#include "platform/sthreads.h"

#if defined(SPLATFORM_THREADS_WINDOWS)

    #include <Windows.h>

/**
 * @brief Get the size of sThread.
 *
 * @return Size of sThread
 */
u64 sThreadSize(void) {
    return sizeof(sThread);
}

/**
 * @brief Create and start a new thread.
 *
 * To get the size of sThread use sThreadSize function.
 *
 * @param thread Pointer to the thread
 * @param func The function that thread runs
 * @param data The data passed as paramter to the function
 *
 * @return Returns true on success, else false.
 */
b8 sThreadCreate(sThread *thread, sThread_func func, void *data) {
    *thread =
        (sThread){.handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func,
                                         data, 0, NULL)};
    return thread->handle ? true : false;
}

/**
 * @brief Wait till the given thread completes.
 *
 * @param thread The thread to join
 *
 * @return Returns the exit code (return value) of thread.
 */
u64 sThreadJoin(sThread thread) {
    WaitForSingleObject(thread.handle, INFINITE);
    u64 exitcode;
    if (!GetExitCodeThread(thread.handle, (DWORD *)&exitcode)) exitcode = -1;
    CloseHandle(thread.handle);
    thread.handle = NULL;
    return exitcode;
}

/**
 * @brief Exit with exitcode.
 *
 * Terminates the calling thread.
 *
 * @param exitcode The exit code
 */
void sThreadExit(u64 exitcode) {
    ExitThread(exitcode);
}

/**
 * @brief Get the current thread.
 *
 * @return Current sThread
 */
sThread sThreadCurrent(void) {
    return (sThread){.handle = GetCurrentThread()};
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
    return TerminateThread(thread.handle, exitcode);
}

#endif
