#pragma once

#include "defines.h"

#ifndef ASSERTIONS_ENABLED
    #define ASSERTIONS_ENABLED 1
#endif

#ifdef S_RELEASE
    #define ASSERTIONS_ENABLED 0
#endif

SAPI void reportAssertionFailure(const char *expr, const char *msg,
                                 const char *file, const i32 line);

#if ASSERTIONS_ENABLED == 1

    #ifdef _MSC_VER
        #include <intrin.h>
        #define DEBUG_BREAK __debugbreak()
    #else
        #if __has_builtin(__builtin_debugtrap)
            #define DEBUG_BREAK __builtin_debugtrap()
        #else
            #define DEBUG_BREAK __builtin_trap()
        #endif
    #endif

    #define SASSERT_MSG(expr, msg)                                  \
        if (expr) {                                                 \
        } else {                                                    \
            reportAssertionFailure(#expr, msg, __FILE__, __LINE__); \
            DEBUG_BREAK;                                            \
        }

    #define SASSERT(expr) SASSERT_MSG(expr, "")

#else

    #define SASSERT_MSG(expr, msg)
    #define SASSERT(expt)

#endif
