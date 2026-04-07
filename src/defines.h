#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#define SNUK_UNUSED(x) ((void)(x))

#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))

#if defined(SNUK_DEBUG)
#define SNUK_ASSERT(cond, msg) assert((cond) && msg)
#else
#define SNUK_ASSERT(cond, msg)
#endif

#define SNUK_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)

#define SNUK_INLINE static inline

#define SNUK_STRINGIFY(x) #x
