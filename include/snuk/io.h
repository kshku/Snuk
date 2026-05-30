#pragma once

#include "defines.h"

// reads one line from stdin, caller should free
SNUK_API char *snuk_read_line(char *buffer, uint64_t size);

// reads entire file, caller should free
SNUK_API char *snuk_read_file(const char *path);

SNUK_API void snuk_print(const char *fmt, ...);

SNUK_API void snuk_println(const char *fmt, ...);

// stderr, for error output
SNUK_API void snuk_eprint(const char *fmt, ...);

// stderr
SNUK_API void snuk_eprintln(const char *fmt, ...);
