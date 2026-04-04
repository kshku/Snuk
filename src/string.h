#pragma once

#include "defines.h"

SNUK_INLINE uint64_t snuk_string_length(const char *s) {
    if (!s) return 0;

    uint64_t len = 0;
    for (len = 0; s[len]; ++len);
    return len;
}

SNUK_INLINE bool snuk_string_equal(const char *a, const char *b) {
    if (!a || !b) return false;

    uint64_t i = 0;
    for (i = 0; a[i] && b[i] && a[i] == b[i]; ++i);

    if (a[i] || b[i]) return false;

    return true;
}

SNUK_INLINE bool snuk_string_n_equal(const char *a, const char *b, uint64_t n) {
    if (!a || !b) return false;

    uint64_t i = 0;
    for (i = 0; a[i] && b[i] && a[i] == b[i]; ++i);

    return i == n;
}
