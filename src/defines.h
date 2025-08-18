#pragma once

// https://sourceforge.net/p/predef/wiki/Home/
#if defined(__clang__)
    #define SNUK_COMPILER_CLANG
#elif defined(__GNUC__)
    #define SNUK_COMPILER_GCC
#elif defined(_MSC_VER)
    #define SNUK_COMPILER_MSVC
#else
    #error "Compiler not identified"
#endif

#if defined(__gnu_linux__) || defined(__linux__)
    #define SNUK_OS_LINUX
#elif defined(_WIN64)
    #define SNUK_OS_WINDOWS
#elif defined(macintosh) || defined(Macintosh) \
    || (defined(__APPLE__) && defined(__MACH__))
    #define SNUK_OS_MAC
#else
    #error "Operating system not identified"
#endif

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) \
    || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
    #define SNUK_ARCH_AMD64
#elif defined(__aarch64__)
    #define SNUK_ARCH_ARM64
#else
    #error "Unsupported architecture"
#endif

#define SNUK_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)

#ifdef SNUK_COMPILER_MSVC
    #define SNUK_FORCE_INLINE static __forceinline
#else
    #define SNUK_FORCE_INLINE static inline __attribute__((always_inline))
#endif

#define SNUK_INLINE static inline

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef int_fast8_t if8;
typedef int_fast16_t if16;
typedef int_fast32_t if32;
typedef int_fast64_t if64;

typedef int_least8_t il8;
typedef int_least16_t il16;
typedef int_least32_t il32;
typedef int_least64_t il64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef uint_fast8_t uf8;
typedef uint_fast16_t uf16;
typedef uint_fast32_t uf32;
typedef uint_fast64_t uf64;

typedef uint_least8_t ul8;
typedef uint_least16_t ul16;
typedef uint_least32_t ul32;
typedef uint_least64_t ul64;

typedef _Bool b8;

SNUK_STATIC_ASSERT(sizeof(b8) == 1, "Expected bool to be 1 byte");
SNUK_STATIC_ASSERT(sizeof(void *) == 8, "Expected void * to be 8 byte");

#define SNUK_UNUSED(x) ((void)(x))

#define SNUK_STRINGIFY(x) #x

#define SNUK_STRINGIFY_EXPANDED(x) SNUK_STRINGIFY(x)

#define SNUK_CONCAT(x, y) x##y

#define SNUK_CONCAT3(x, y, z) x##y##z

#define SNUK_CONCAT_EXPANDED(x, y) SNUK_CONCAT(x, y)

#define SNUK_CONCAT_EXPANDED3(x, y, z) SNUK_CONCAT3(x, y, z)

#define SNUK_BITFLAG(n) (1 << (n))

#define SNUK_BIT_SET(x, n) ((x) |= SNUK_BITFLAG((n)))

#define SNUK_BIT_CLEAR(x, n) ((x) &= ~SNUK_BITFLAG((n)))

#define SNUK_BIT_TOGGLE(x, n) ((x) ^= SNUK_BITFLAG((n)))

#define SNUK_BIT_CHECK(x, n) ((x) & SNUK_BITFLAG((n)))
