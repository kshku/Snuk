#pragma once

#include "atomic.h"
#include "defines.h"
#include "utils.h"

/**
 * @brief Spinlock type.
 */
typedef snuk_atomic_flag snuk_spinlock;

/**
 * @brief Initialize the spinlock.
 */
#define SNUK_SPINLOCK_INIT SNUK_ATOMIC_FLAG_INIT

/**
 * @brief Get the lock.
 */
#define snuk_spinlock_lock(lock)                                              \
    while (snuk_atomic_flag_test_and_set_explicit(lock,                       \
                                                  SNUK_MEMORY_ORDER_ACQUIRE)) \
        SNUK_PAUSE_INSTRUCTION;

/**
 * @brief Try to get the lock.
 *
 * @return Returns true if lock is acquired, else false.
 */
#define snuk_spinlock_try_lock(lock) \
    !snuk_atomic_flag_test_and_set_explicit(lock, SNUK_MEMORY_ORDER_ACQUIRE)

/**
 * @brief Release the lock.
 */
#define snuk_spinlock_unlock(lock) \
    snuk_atomic_flag_clear_explicit(lock, SNUK_MEMORY_ORDER_RELEASE)
