#pragma once

#include "defines.h"

// reads one line from stdin, caller should free
char *snuk_read_line(void);

// reads entire file, caller should free
char *snuk_read_file(const char *path);

void snuk_free_file(char *ptr);

void snuk_free_line(char *ptr);

void snuk_print(const char *fmt, ...);

void snuk_println(const char *fmt, ...);

// stderr, for error output
void snuk_eprint(const char *fmt, ...);

// stderr
void snuk_eprintln(const char *fmt, ...); 
