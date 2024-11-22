#include "logger.h"

#include <stdarg.h>

#include "assertions.h"
#include "platform/log.h"

/**
 * @brief Initializes the logger.
 *
 * @param file The file to log to
 *
 * @return true if the logger was initialized successfully.
 */
b8 initializeLogger(const char *file) {
    UNUSED(file);
    // TODO: Logging to file, aynchronous logging, etc.
    return true;
}

/**
 * @brief Shuts down the logger.
 */
void shutdownLogger(void) {
}

/**
 * @brief Logs a message.
 *
 * @param level Log level (one of the values of enum LogLevel)
 * @param msg Format string message
 * @param ... Arguments for the format string
 */
void _logMessage(LogLevel level, const char *msg, ...) {
    static const char *log_level_strings[] = {
        "[FATAL]: ", "[ERROR]: ", "[WARN]: ",
        "[INFO]: ",  "[DEBUG]: ", "[TRACE]: "};

    // TODO: Log to file, timestamp, etc.
    va_list args;
    va_start(args, msg);
    platformLogMessage(level, msg, args, log_level_strings[level]);
    va_end(args);
}

/**
 * @brief Report the assertion failure.
 *
 * @param expr The expression that failed
 * @param msg The message to be printed
 * @param file The file in which assertion failed
 * @param line Line number where the assertion failed
 */
void _reportAssertionFailure(const char *expr, const char *msg,
                             const char *file, const i32 line) {
    sFatal("Assertion failure: '%s', message '%s' in %s:%d", expr, msg, file,
           line);
}
