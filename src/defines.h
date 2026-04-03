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
