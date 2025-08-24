#pragma once

#include "defines.h"

#ifdef SNUK_ARCH_AMD64
    #ifdef SNUK_COMPILER_MSVC
void snuk_utils_pause_instruction(void);
void snuk_utils_debug_break(void);

        #define SNUK_PAUSE_INSTRUCTION snuk_utils_pause_instruction()
        #define SNUK_DEBUG_BREAK snuk_utils_debug_break()
    #else
        #define SNUK_PAUSE_INSTRUCTION __asm__ volatile("pause")
        #define SNUK_DEBUG_BREAK __asm__ volatile(".byte 0xCC") /* int 3 */
    #endif

#endif
