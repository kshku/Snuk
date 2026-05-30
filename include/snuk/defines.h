#pragma once

#include "api.h"
#include "logger.h"
#include "platform.h"

#include <assert.h>
#include <inttypes.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SNUK_UNUSED(x) ((void)(x))

#define SNUK_ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

#if defined(SNUK_DEBUG)
    #if defined(SNUK_COMPILER_GCC) || defined(SNUK_COMPILER_CLANG)
        #define SNUK_ASSERT(cond, msg)    \
            do {                          \
                if (!(cond)) {            \
                    log_fatal("%s", msg); \
                    __builtin_trap();     \
                }                         \
            } while (0)
    #else
        #define SNUK_ASSERT(cond, msg)    \
            do {                          \
                if (!(cond)) {            \
                    log_fatal("%s", msg); \
                    __debugbreak();       \
                }                         \
            } while (0)
    #endif
#else
    #define SNUK_ASSERT(cond, msg) SNUK_UNUSED(cond), SNUK_UNUSED(msg)
#endif

#define SNUK_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)

#define SNUK_INLINE static inline

#if defined(SNUK_COMPILER_MSVC)
    #define SNUK_FORCE_INLINE static __forceinline
#else
    #define SNUK_FORCE_INLINE static inline __attribute__((always_inline))
#endif

#define SNUK_STRINGIFY(x) #x

#define SNUK_SHOULD_NOT_REACH_HERE SNUK_ASSERT(false, "Should not reach here")
