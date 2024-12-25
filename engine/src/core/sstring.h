#pragma once

#include "defines.h"

SAPI u64 sStringLenght(const char *str);

SAPI char *sStringConcat(const char *str1, const char *str2, u64 len1, u64 len2,
                         u64 *length);

SAPI b8 sStringEqual(const char *str1, const char *str2);

SAPI char *sStringCopy(const char *str, u64 len);
