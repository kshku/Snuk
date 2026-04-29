#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdalign.h>

#define SNUK_UNUSED(x) ((void)(x))

#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))

#if defined(SNUK_DEBUG)
#define SNUK_ASSERT(cond, msg) assert((cond) && msg)
#else
#define SNUK_ASSERT(cond, msg) SNUK_UNUSED(cond), SNUK_UNUSED(msg)
#endif

#define SNUK_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)

#define SNUK_INLINE static inline

#define SNUK_STRINGIFY(x) #x

#define SNUK_SHOULD_NOT_REACH_HERE SNUK_ASSERT(false, "Should not reach here")
