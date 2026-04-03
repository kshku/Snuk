#include "logger.h"

#include <snlogger/snlogger.h>

#include <stdlib.h>
#include <stdio.h>

#if defined(SN_OS_WINDOWS)
#include <windows.h>

static bool enableVTProcessing(DWORD handle_type);
#else
#include <unistd.h>
#endif

#define LOGGER_BUFFER_SIZE 1024

typedef struct stdout_stderr_sink {
    // 0 -> stdout, 1 -> stderr
    bool colored_enabled[2];
} stdout_stderr_sink;

static void stdout_stderr_sink_write(const char *msg, size_t len, snLogLevel level, void *data);
static void stdout_stderr_sink_open(void *data);
static void stdout_stderr_sink_flush(void *data);


static snStaticLogger sl;
static char log_buffer[LOGGER_BUFFER_SIZE];
static stdout_stderr_sink sink_data;
static snSink sinks[] = {
    (snSink){
        .open = stdout_stderr_sink_open,
        .flush = stdout_stderr_sink_flush,
        .write = stdout_stderr_sink_write,
        .data = &sink_data
    }
};

void snuk_logger_init(void) {
    sn_static_logger_init(&sl, log_buffer, LOGGER_BUFFER_SIZE, sinks, ARRAY_LEN(sinks));
}

void snuk_logger_deinit(void) {
    sn_static_logger_deinit(&sl);
}

void log_msg(snLogLevel level, const char *file, const char *function, long line, const char *format_string, ...) {
    if (level < sl.level) return;

    const char *level_string = NULL;
    const char *level_strings[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
    level_string = level_strings[level];


    char buffer[1024] = {0};
    size_t buffer_size = ARRAY_LEN(buffer);
    size_t len = 0;

    if (level < SN_LOG_LEVEL_WARN)
        len = snprintf(buffer, buffer_size, "[%s]: %s\n", level_string, format_string);
    else
        len = snprintf(buffer, buffer_size, "[%s]: %s:%lu in function %s: %s\n", 
                level_string, file, line, function, format_string);

    if (len < 0) abort();

    if (len >= buffer_size) {
        log_msg(SN_LOG_LEVEL_ERROR, file, function, line, "Too long message, try increasing the buffer size or decreasing message length!");
        return;
    }

    buffer[len] = 0;

    va_list args;
    va_start(args, format_string);
    sn_static_logger_log_va(&sl, level, buffer, args);
    va_end(args);
}

static void stdout_stderr_sink_write(const char *msg, size_t len, snLogLevel level, void *data) {
    stdout_stderr_sink *sink = (stdout_stderr_sink *)data;

    bool error = level > SN_LOG_LEVEL_WARN;
    FILE *out_file = error ? stderr : stdout;

    // error true => 1 => stderr
    // error false => 0 => stdout
    if (sink->colored_enabled[(int)error]) {
        switch (level) {
            case SN_LOG_LEVEL_TRACE:
                fputs("\x1b[0;37m", out_file);
                break;
            case SN_LOG_LEVEL_DEBUG:
                fputs("\x1b[0;34m", out_file);
                break;
            case SN_LOG_LEVEL_INFO:
                fputs("\x1b[0;32m", out_file);
                break;
            case SN_LOG_LEVEL_WARN:
                fputs("\x1b[0;33m", out_file);
                break;
            case SN_LOG_LEVEL_ERROR:
                fputs("\x1b[1;31m", out_file);
                break;
            case SN_LOG_LEVEL_FATAL:
                fputs("\x1b[1;41m", out_file);
                break;
        }
    }

    fwrite(msg, sizeof(char), len, out_file);

    if (sink->colored_enabled[(int)error]) fputs("\x1b[0m", out_file);
}

static void stdout_stderr_sink_open(void *data) {
    stdout_stderr_sink *sink = (stdout_stderr_sink *)data;
#if defined(SN_OS_WINDOWS)
    sink->colored_enabled[0] = enableVTProcessing(STD_OUTPUT_HANDLE);
    sink->colored_enabled[1] = enableVTProcessing(STD_ERROR_HANDLE);
#else
    sink->colored_enabled[0] = isatty(STDOUT_FILENO);
    sink->colored_enabled[1] = isatty(STDERR_FILENO);
#endif
}

static void stdout_stderr_sink_flush(void *data) {
    (void)data;
    fflush(stdout);
    fflush(stderr);
}

#if defined(SN_OS_WINDOWS)
static bool enableVTProcessing(DWORD handle_type) {
    HANDLE handle = GetStdHandle(handle_type);
    if (handle == INVALID_HANDLE_VALUE) return false;

    DWORD modes = 0;
    if (!GetConsoleMode(handle, &modes)) return false;

    modes |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING
           | DISABLE_NEWLINE_AUTO_RETURN;
    if (!SetConsoleMode(handle, modes)) return false;

    return true;
}
#endif


