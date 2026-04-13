#pragma once

#include "defines.h"

// reads one line from stdin, caller should free
char *snuk_read_line(char *buffer, uint64_t size);

// reads entire file, caller should free
char *snuk_read_file(const char *path);

void snuk_print(const char *fmt, ...);

void snuk_println(const char *fmt, ...);

// stderr, for error output
void snuk_eprint(const char *fmt, ...);

// stderr
void snuk_eprintln(const char *fmt, ...); 
