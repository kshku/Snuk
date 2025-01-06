#pragma once

#include "defines.h"

SAPI u64 sStringLengthC8(const c8 *str);

SAPI c8 *sStringConcatC8(const c8 *str1, const c8 *str2, u64 l1, u64 l2,
                         u64 *length);

SAPI b8 sStringEqualC8(const c8 *str1, const c8 *str2, u64 len);

SAPI c8 *sStringCopyC8(const c8 *str, u64 len);

SAPI u64 sStringLengthC16(const c16 *str);

SAPI c16 *sStringConcatC16(const c16 *str1, const c16 *str2, u64 l1, u64 l2,
                           u64 *length);

SAPI b8 sStringEqualC16(const c16 *str1, const c16 *str2, u64 len);

SAPI c16 *sStringCopyC16(const c16 *str, u64 len);
