#pragma once

// Platform detection
#if defined(__WIN32__) || defined(_WIN32)
    #define SPLATFORM_OS_WINDOWS
    #ifndef _WIN64
        #error "64-bit is required on Windows"
    #endif
#elif defined(__linux__) || defined(__gnu_linux__)
    #define SPLATFORM_OS_LINUX
#else
    #error "Platform not supported"
#endif

// SAPI for exporting and importing
#ifdef S_EXPORTS
    #ifdef SPLATFORM_OS_WINDOWS
        #define SAPI __declspec(dllexport)
    #else
        #define SAPI __attribute__((visibility("default")))
    #endif
#else
    #ifdef SPLATFORM_OS_WINDOWS
        #define SAPI __declspec(dllimport)
    #else
        #define SAPI
    #endif
#endif

// signed integer types
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

// unsigned integer types
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// floating point types
typedef float f32;
typedef double f64;

// boolean type
typedef _Bool b8;

#define true 1
#define false 0

// null
#define NULL ((void *)0)

// Static assert
#if defined(__clang__) || defined(__GNUC__)
    #define STATIC_ASSERT _Static_assert
#else
    #define STATIC_ASSERT static_assert
#endif

// Make sure the sizes of the types are correct
STATIC_ASSERT(sizeof(i8) == 1, "Expected i8 to be 1 byte");
STATIC_ASSERT(sizeof(i16) == 2, "Expected i16 to be 2 bytes");
STATIC_ASSERT(sizeof(i32) == 4, "Expected i32 to be 4 bytes");
STATIC_ASSERT(sizeof(i64) == 8, "Expected i64 to be 8 bytes");

STATIC_ASSERT(sizeof(u8) == 1, "Expected u8 to be 1 byte");
STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 to be 2 bytes");
STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 to be 4 bytes");
STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 to be 8 bytes");

STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 bytes");
STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 bytes");

STATIC_ASSERT(sizeof(b8) == 1, "Expected b8 to be 1 byte");

/**
 * @brief Mark variables, return values as unused
 *
 * Marking variables, return values as unused says that the variable is
 * intentionally unused.
 *
 * @param x Variable of function call
 */
#define UNUSED(x) (void)(x)

/**
 * @brief Convert the input to string.
 *
 * @param x value to be converted to string
 */
#define STRINGIFY(x) #x

/**
 * @brief Join two inputs.
 *
 * @param x first one
 * @param y second one
 */
#define CONCAT(x, y) x##y

/**
 * @brief Expands and then joins.
 *
 * Usefull when we are passing macro as an argument and we want the macro to be
 * expanded before the concatenation.
 *
 * @param x first one
 * @param y second one
 */
#define CONCAT_EXPANDED(x, y) CONCAT(x, y)

/**
 * @brief Bit flags.
 *
 * @param n Number of shifts to 1
 */
#define BITFLAG(n) (1 << n)
