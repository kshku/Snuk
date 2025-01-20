#pragma once

#include "defines.h"

typedef struct SString {
        char *str;
        u64 size;
} SString;

SAPI void sStringCreate(SString *sstr, const char *str, u64 size);

SAPI u64 sStringLength(const char *str);

SAPI char *sStringConcat(const char *str1, const char *str2, u64 l1, u64 l2,
                         u64 *length);

SAPI b8 sStringEqual(const char *str1, const char *str2, u64 len);

SAPI char *sStringCopy(const char *str, u64 len);

// SAPI u64 sStringLengthC16(const c16 *str);

// SAPI c16 *sStringConcatC16(const c16 *str1, const c16 *str2, u64 l1, u64 l2,
//                            u64 *length);

// SAPI b8 sStringEqualC16(const c16 *str1, const c16 *str2, u64 len);

// SAPI c16 *sStringCopyC16(const c16 *str, u64 len);
