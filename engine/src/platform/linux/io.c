#include "../io.h"

#ifdef SPLATFORM_LINUX

    #include <stdio.h>
    #include <unistd.h>

/**
 * @brief Logging helper for Linux.
 *
 * This function uses ANSI escape sequences to provide colored output.
 * If level was fatal or error, flush and write message immediatly to stderr
 * using dprintf(which is unbuffered). Else write message to stdout using
 * printf(which is buffered).
 *
 * @param level Log level
 * @param msg The message to write
 *
 * @note Didn't got any bugs while printing multi-line messages. So
 * not handling multi-line messages by printing them line by line with
 * respective style.
 */
void platformLogMessage(LogLevel level, const char *msg) {
    // https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797#colors--graphics-mode
    static const char *colors[6] = {
        "1;41", "1;31", "0;33",  // Fatal, Error, Warn
        "0;32", "0;34", "0;37"};  // Info, Debug, Trace

    if (!isatty(STDERR_FILENO)) {  // Not a terminal
        fprintf(stderr, "%s\n", msg);
    } else {
        fprintf(level < LOG_LEVEL_WARN ? stderr : stdout, "\x1b[%sm%s\x1b[0m\n",
                colors[level], msg);
    }

    if (level < LOG_LEVEL_WARN) fflush(NULL);  // Fatal and Error
}

#endif
