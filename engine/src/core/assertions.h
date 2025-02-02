#pragma once

#include "defines.h"

#ifndef ASSERTIONS_ENABLED
    #define ASSERTIONS_ENABLED 1
#endif

#ifdef S_RELEASE
    #define ASSERTIONS_ENABLED 0
#endif

SAPI void _reportAssertionFailure(const char *restrict expr,
                                  const char *restrict msg,
                                  const char *restrict file, const i32 line);

#if ASSERTIONS_ENABLED == 1

    #ifdef SPLATFORM_ARCH_X86_64
        /**
         * @brief Put a debug break point to stop execution.
         */
        #define DEBUG_BREAK \
            __asm__ volatile(".byte 0xCC") /* __asm__("int $3") */
    #else
        #ifdef _MSC_VER
            #include <intrin.h>
            /**
             * @brief Put a debug break point to stop execution.
             */
            #define DEBUG_BREAK __debugbreak()
        #else
            #if __has_builtin(__builtin_debugtrap)
                /**
                 * @brief Put a debug break point to stop execution.
                 */
                #define DEBUG_BREAK __builtin_debugtrap()
            #else
                /**
                 * @brief Put a illegal statement to stop execution.
                 */
                #define DEBUG_BREAK __builtin_trap()
            #endif
        #endif
    #endif

    /**
     * @brief Assert expression is true.
     *
     * If assertion is false then report the assertion failure with the given
     * message and then put a break point.
     *
     * @param expr Expression to be asserted true
     * @param msg Message to be printed when assertion fails
     */
    #define sassert_msg(expr, msg)                                       \
        do {                                                             \
            if (expr) {                                                  \
            } else {                                                     \
                _reportAssertionFailure(#expr, msg, __FILE__, __LINE__); \
                DEBUG_BREAK;                                             \
            }                                                            \
        } while (0)

    /**
     * @brief Assert expression is true.
     *
     * If assertion is false then report the assertion failure and then put a
     * break point.
     *
     * @param expr Expression to be asserted true
     */
    #define sassert(expr) sassert_msg(expr, "")

#else

    #define sassert_msg(expr, msg)
    #define sassert(expt)

#endif
