#pragma once

#include "defines.h"

#include "snuk_string.h"
#include "memory.h"
#include <string.h>

#define SNUK_STRING_VIEW_FORMAT "%.*s"
#define SNUK_STRING_VIEW_ARG(sv) (sv).len, (sv).str

typedef struct SnukStringView {
    const char *str;
    uint64_t len;
} SnukStringView;

SNUK_INLINE SnukStringView snuk_string_view_create_with_len(const char *str, uint64_t len) {
    return (SnukStringView){
        .str = str,
        .len = len,
    };
}

SNUK_INLINE SnukStringView snuk_string_view_create(const char *str) {
    return snuk_string_view_create_with_len(str, string_length(str));
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
