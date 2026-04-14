#include "io.h"

#include <snfile/snfile.h>
#include <stdarg.h>
#include <stdio.h>

#include "memory.h"
#include "logger.h"

// reads one line from stdin, caller should free
char *snuk_read_line(char *buffer, uint64_t size) {
    return fgets(buffer, size, stdin);
}

// reads entire file, caller should free
char *snuk_read_file(const char *path) {
    snFile file;

    if (!sn_file_open(path, SN_FILE_OPEN_FLAG_READ, &file)) return NULL;

    uint64_t file_size = sn_file_size(&file);
    char *content = (char *)snuk_alloc(file_size, alignof(char));

    if (sn_file_read(&file, content, file_size) != (int64_t)file_size)
        log_warn("file_size != read_size", NULL);

    sn_file_close(&file);

    return content;
}

void snuk_print(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
}

void snuk_println(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fputc('\n', stdout);
}

// stderr, for error output
void snuk_eprint(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

// stderr
void snuk_eprintln(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
}
