#pragma once

#include "defines.h"

SNUK_INLINE uint64_t string_length(const char *s) {
    if (!s) return 0;

    uint64_t len = 0;
    for (len = 0; s[len]; ++len);
    return len;
}

SNUK_INLINE bool string_equal(const char *a, const char *b) {
    if (!a || !b) return false;

    uint64_t i = 0;
    for (i = 0; a[i] && b[i] && a[i] == b[i]; ++i);

    if (a[i] || b[i]) return false;

    return true;
}

SNUK_INLINE bool string_equal_ignore_case(const char *a, const char *b) {
    if (!a || !b) return false;

    uint64_t i = 0;
    for (i = 0; a[i] && b[i] && (a[i] | (1 << 5)) == (b[i] | (1 << 5)); ++i);

    if (a[i] || b[i]) return false;

    return true;
}

SNUK_INLINE bool string_n_equal(const char *a, const char *b, uint64_t n) {
    if (!a || !b) return false;

    uint64_t i = 0;
    for (i = 0; a[i] && b[i] && a[i] == b[i]; ++i);

    return i == n;
}

SNUK_INLINE bool string_n_equal_ignore_case(const char *a, const char *b, uint64_t n) {
    if (!a || !b) return false;

    uint64_t i = 0;
    for (i = 0; a[i] && b[i] && (a[i] | (1 << 5)) == (b[i] | (1 << 5)); ++i);

    return i == n;
}

SNUK_INLINE bool char_in_string(char c, const char *s) {
    if (!s) return false;
    for (uint64_t i = 0; s[i] || i == 0; ++i) if (s[i] == c) return true;
    return false;
}
