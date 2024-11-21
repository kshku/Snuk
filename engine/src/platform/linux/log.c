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
 * @param msg The message to write
 */
void platformLogMessage(LogLevel level, const char *msg) {
    static const char *colors[6] = {
        "1;41", "1;31", "0;33",  // Fatal, Error, Warn
        "0;32", "0;34", "0;37"};  // Info, Debug, Trace

    if (level < LOG_LEVEL_WARN) {  // Fatal and Error
        if (!isatty(STDERR_FILENO)) fprintf(stderr, "%s\n", msg);
        else fprintf(stderr, "\x1b[%sm%s\x1b[0m\n", colors[level], msg);
        fflush(NULL);
    } else {
        if (!isatty(STDOUT_FILENO)) fprintf(stdout, "%s\n", msg);
        else fprintf(stdout, "\x1b[%sm%s\x1b[0m\n", colors[level], msg);
    }
}

#endif
