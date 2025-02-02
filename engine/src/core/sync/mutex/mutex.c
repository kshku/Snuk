#include "mutex.h"

#include <stdatomic.h>

#include "core/logger.h"

void sMutexInit(smutex *mutex) {
    atomic_store_explicit(&mutex->mutex, false, memory_order_release);
}

void sMutexLock(smutex *mutex) {
    b8 expected = false;
    while (!atomic_compare_exchange_weak_explicit(&mutex->mutex, &expected,
                                                  true, memory_order_acquire,
                                                  memory_order_relaxed)) {
        expected = false;
#ifdef SPLATFORM_ARCH_X86_64
        __asm__ volatile("pause");
#endif
    }
}

b8 sMutexTryLock(smutex *mutex) {
    b8 expected = false;
    return atomic_compare_exchange_strong_explicit(&mutex->mutex, &expected,
                                                   true, memory_order_acquire,
                                                   memory_order_relaxed);
}

void sMutexUnlock(smutex *mutex) {
    atomic_store_explicit(&mutex->mutex, false, memory_order_release);
}
