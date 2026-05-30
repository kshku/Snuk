#pragma once

#include "platform.h"

#if defined(SNUK_STATIC)
    #define SNUK_API
#else
    #if defined(SNUK_EXPORT)
        #if defined(SNUK_OS_LINUX) || defined(SNUK_OS_MAC)
            #define SNUK_API __attribute__((visibility("default")))
        #elif defined(SNUK_OS_WINDOWS)
            #define SNUK_API __declspec(dllexport)
        #else
            #error "Should not reach here!"
        #endif
    #else
        #if defined(SNUK_OS_LINUX) || defined(SNUK_OS_MAC)
            #define SNUK_API
        #elif defined(SNUK_OS_WINDOWS)
            #define SNUK_API __declspec(dllimport)
        #else
            #error "Should not reach here!"
        #endif
    #endif
#endif
