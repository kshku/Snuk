#pragma once

// Compiler
#if defined(__clang__)
    #define SNUK_COMPILER_CLANG
#elif defined(__GNUC__)
    #define SNUK_COMPILER_GCC
#elif defined(_MSC_VER)
    #define SNUK_COMPILER_MSVC
#else
    #error "Don't know whether works on this compiler!"
#endif

// Operating system
#if defined(__gnu_linux__) || defined(__linux__)
    #define SNUK_OS_LINUX
#elif defined(_WIN64)
    #define SNUK_OS_WINDOWS
#elif defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
    #define SNUK_OS_MAC
#else
    #error "Don't know whether works on this OS!"
#endif

