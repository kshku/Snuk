#pragma once

#include <stddef.h>
#include <stdbool.h>

#define SNUK_UNUSED(x) ((void)(x))

#define ARRAY_LEN(x) (sizeof(x) / sizeof(x[0]))
