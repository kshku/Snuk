#include "../log.h"

#ifdef SPLATFORM_LINUX

    #include <stdio.h>
    #include <unistd.h>

/**
 * @brief Logging helper implementation of Linux.
 *
 * This function uses the ANSI escape sequences to provide colored output. If
 * level was fatal or error, writes to stderr else writes to stdout. If standard
 * ouput or error are not terminal, then message will be written to stdout and
 * stderr without any escape sequences.
 *
 * https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797#colors--graphics-mode
 *
 * @param level Log level
 * @param msg The format string message to write
 * @param args Arguments to the format string message
 * @param prefix Format string prefix to be displayed before the message
 * @param ... Arguments to the format string prefix
 *
 * @note The prefix parameter is used so that the logger don't have to allocate
 * memory and do the formatting to add the prefix to the message sent by the
 * user.
 */
void platformLogMessage(LogLevel level, const char *msg, va_list args,
                        const char *prefix, ...) {
    static const char *colors[6] = {
        "1;41", "1;31", "0;33",  // Fatal, Error, Warn
        "0;32", "0;34", "0;37"};  // Info, Debug, Trace

    // Track whether we are writitng to terminal or not
    // 0 -> stdout 1 -> stderr
    static b8 is_terminal = {false, false};

    // true = 1 and false = 0
    // Using u8 just because it is used to access the elements of is_terminal.
    u8 error = level < LOG_LEVEL_WARN;  // Fatal or Error is error
    FILE *out_file = error ? stderr : stdout;

    // TODO: Since files will point to the same untill the application ends,
    // TODO: only need to check whether we are writing to terminal. So do this
    // TODO: only once.
    if (!is_terminal[error])
        is_terminal[error] = isatty(error ? STDERR_FILENO : STDOUT_FILENO);

    // If terminal, we can print color
    if (is_terminal[error]) fprintf(out_file, "\x1b[%sm", colors[level]);

    va_list parg;
    va_start(parg, prefix);
    vfprintf(out_file, prefix, parg);
    va_end(parg);

    vfprintf(out_file, msg, args);

    // If terminal, reset the color after printing the message
    if (is_terminal[error]) fprintf(out_file, "\x1b[0m");

    fprintf(out_file, "\n");

    if (error) fflush(NULL);
}

#endif
