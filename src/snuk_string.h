#pragma once

#include "defines.h"

#include "memory.h"

#include <string.h>

SNUK_INLINE bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

SNUK_INLINE bool is_alpha_numeric(char c) {
    return is_alpha(c) || (c >= '0' && c <= '9');
}

SNUK_INLINE bool is_digit(char c) {
    return (c >= '0' && c <= '9');
}

SNUK_INLINE bool is_binary_digit(char c) {
    return c == '0' || c == '1';
}

SNUK_INLINE bool is_octal_digit(char c) {
    return (c >= '0' && c <= '7');
}

SNUK_INLINE bool is_hex_digit(char c) {
    if (c >= '0' && c <= '9') return true;
    c |= (1 << 5);
    return c >= 'a' && c <= 'f';
}

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
    for (i = 0; a[i] && b[i]; ++i) {
        if (is_alpha(a[i]) && (a[i] | (1 << 5)) != (b[i] | (1 << 5)))
            return false;
        else if (a[i] != b[i])
            return false;
    }

    if (a[i] || b[i]) return false;

    return true;
}

SNUK_INLINE bool string_n_equal(const char *a, const char *b, uint64_t n) {
    if (!a || !b) return false;

    uint64_t i = 0;
    for (i = 0; i < n && a[i] && b[i] && a[i] == b[i]; ++i);

    return i == n;
}

SNUK_INLINE bool string_n_equal_ignore_case(const char *a, const char *b, uint64_t n) {
    if (!a || !b) return false;

    uint64_t i = 0;
    for (i = 0; i < n && a[i] && b[i]; ++i) {
        if (is_alpha(a[i]) && (a[i] | (1 << 5)) != (b[i] | (1 << 5)))
            return false;
        else if (a[i] != b[i])
            return false;
    }

    return i == n;
}

SNUK_INLINE bool char_in_string(char c, const char *s) {
    if (!s) return false;
    for (uint64_t i = 0; s[i] || i == 0; ++i) if (s[i] == c) return true;
    return false;
}

// if len == 0 (of any string), snuk_string_length will be called.
SNUK_INLINE char *snuk_string_concat(const char *a, uint64_t alen, const char *b, uint64_t blen) {
    if (!alen) alen = string_length(a);
    if (!blen) blen = string_length(b);

    char *new = snuk_alloc((alen + blen + 1) * sizeof(char), alignof(char));
    memcpy(new, a, alen);
    memcpy(new + alen, b, blen);
    new[alen + blen] = 0;

    return new;
}
