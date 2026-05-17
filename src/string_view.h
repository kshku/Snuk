#pragma once

#include <string.h>

#include "defines.h"
#include "memory.h"
#include "snuk_string.h"

#define SNUK_STRING_VIEW_FORMAT "%.*s"
#define SNUK_STRING_VIEW_ARG(sv) (sv).len, (sv).str

typedef struct SnukStringView {
        const char *str;
        uint64_t len;
} SnukStringView;

SNUK_INLINE SnukStringView
snuk_string_view_create_with_len(const char *str, uint64_t len) {
    return (SnukStringView){
        .str = str,
        .len = len,
    };
}

SNUK_INLINE SnukStringView snuk_string_view_create(const char *str) {
    return snuk_string_view_create_with_len(str, snuk_string_length(str));
}

SNUK_INLINE char *snuk_string_view_get_cstr(SnukStringView view) {
    char *str = snuk_alloc((view.len + 1) * sizeof(char), alignof(char));
    memcpy(str, view.str, view.len);
    str[view.len] = 0;
    return str;
}

SNUK_INLINE SnukStringView snuk_string_view_copy(SnukStringView view) {
    char *str = snuk_alloc(sizeof(char) * view.len, alignof(char));
    memcpy(str, view.str, view.len);

    return (SnukStringView){
        .str = str,
        .len = view.len,
    };
}

SNUK_INLINE SnukStringView
snuk_string_view_concat(SnukStringView a, SnukStringView b) {
    return (SnukStringView){
        .str = snuk_string_concat(a.str, a.len, b.str, b.len),
        .len = a.len + b.len,
    };
}

SNUK_INLINE bool snuk_string_view_equal(SnukStringView a, SnukStringView b) {
    if (a.len != b.len) return false;
    if (a.len == 0) return true;

    return memcmp(a.str, b.str, a.len) == 0;
}

SNUK_INLINE bool snuk_string_view_equal_ignore_case(
    SnukStringView a, SnukStringView b) {
    if (a.len != b.len) return false;
    if (a.len == 0) return true;

    for (uint64_t i = 0; i < a.len; ++i)
        if (snuk_lower_case(a.str[i]) != snuk_lower_case(b.str[i]))
            return false;
    return true;
}

SNUK_INLINE bool snuk_string_view_equal_cstr(SnukStringView a, const char *b) {
    return snuk_string_view_equal(a, snuk_string_view_create(b));
}

SNUK_INLINE bool snuk_string_view_equal_cstr_ignore_case(
    SnukStringView a, const char *b) {
    return snuk_string_view_equal_ignore_case(a, snuk_string_view_create(b));
}
