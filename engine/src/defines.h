#pragma once

// Predefined macros
// https://sourceforge.net/p/predef/wiki

// Compiler detection
#if defined(__clang__)
    #define SCOMPILER_CLANG
#elif defined(__GNUC__)
    #define SCOMPILER_GCC
#elif defined(_MSC_VER)
    #define SCOMPILER_MSVC
#else
    #error "Compiler is not supported"
#endif

// Platform detection
#if defined(__WIN32__) || defined(_WIN32)
    #define SPLATFORM_OS_WINDOWS
    #ifndef _WIN64
        #error "64-bit is required on Windows"
    #endif
#elif defined(__linux__) || defined(__gnu_linux__)
    #define SPLATFORM_OS_LINUX
#else
    #error "OS not supported"
#endif

// Architecture detection
#if defined(__amd64__) | defined(__amd64) | defined(__x86_64__) \
    | defined(__x86_64) | defined(_M_X64) | defined(_M_AMD64)
    #define SPLATFORM_ARCH_X86_64
#else
    #error "Architecture not supported"
#endif

// Windowing System
#if defined(SPLATFORM_OS_LINUX)
// TODO: to decide between
// #define SPLATFORM_WINDOWING_X11_XLIB
    #define SPLATFORM_WINDOWING_X11_XCB
// #define SPLATFORM_WINDOWING_WAYLAND
#elif defined(SPLATFORM_OS_WINDOWS)
    #define SPLATFORM_WINDOWING_WIN32
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

// ? Use size_t and ptrdiff_t?
// #include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

// signed integer types
// Exact
// #define aligned_type(type, align) type __attribute__((aligned(align)))
// typedef aligned_type(int8_t, 8) i8;
// typedef int16_t __attribute__((aligned(16))) i16;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
// Fastest
typedef int_fast8_t if8;
typedef int_fast16_t if16;
typedef int_fast32_t if32;
typedef int_fast64_t if64;
// Smallest
typedef int_least8_t il8;
typedef int_least16_t il16;
typedef int_least32_t il32;
typedef int_least64_t il64;
// Maximum width
typedef intmax_t imax;
// Type capable of holding a pointer
typedef intptr_t iptr;

// unsigned integer types
// Exact
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
// Fastest
typedef uint_fast8_t uf8;
typedef uint_fast16_t uf16;
typedef uint_fast32_t uf32;
typedef uint_fast64_t uf64;
// Smallest
typedef uint_least8_t ul8;
typedef uint_least16_t ul16;
typedef uint_least32_t ul32;
typedef uint_least64_t ul64;
// Maximum width
typedef uintmax_t umax;
// Type capable of holding a pointer
typedef uintptr_t uptr;

// floating point types
typedef float f32;
typedef double f64;

// character types
// typedef char c8;
// typedef unsigned short c16;

// boolean type
typedef _Bool b8;

#define true 1
#define false 0

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

// These are guaranteed right?
STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 bytes");
STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 bytes");

// _Bool is not required to be 1 byte but most commonly 1 byte
// Typecasting to bool is different from typecasting to integers
// (bool)0.5 is true where as (int)0.5 is 0
// TODO: Don't want to stop compiling and running just because boolean is not 8
// bits. Doing this only becasue in event Context we have b8 array with 16 size.
// May be just don't let the user pass boolean values through Context
STATIC_ASSERT(sizeof(b8) == 1, "Expected b8 to be 1 byte");

STATIC_ASSERT(sizeof(size_t) <= 8, "size_t > 8 bytes!");

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
 * @brief Join three inputs.
 *
 * @param x first one
 * @param y second one
 * @param z third one
 */
#define CONCAT3(x, y, z) x##y##z

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
 * @brief Expands and then joins.
 *
 * Usefull when we are passing macro as an argument and we want the macro to be
 * expanded before the concatenation.
 *
 * @param x first one
 * @param y second one
 * @param z third one
 */
#define CONCAT_EXPANDED3(x, y, z) CONCAT3(x, y, z)

/**
 * @brief Bit flags.
 *
 * @param n Number of shifts to 1
 */
#define BITFLAG(n) (1 << n)

/**
 * @brief Min of two.
 *
 * @param a First
 * @param b second
 */
#define MIN(a, b) (a < b ? a : b)

/**
 * @brief Max of two.
 *
 * @param a First
 * @param b second
 */
#define MAX(a, b) (a > b ? a : b)
