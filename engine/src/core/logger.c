#include "logger.h"

#include <stdarg.h>
#include <stdio.h>

#include "assertions.h"
#include "platform/log.h"
#include "platform/memory.h"

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
    u64 size = vsnprintf(NULL, 0, msg, args)
             + 10;  // log_level_string require at max 9 char, 1 for null.
    va_end(args);

    // Can't use memory subsystem since it calls logger.
    char *buf = (char *)platformAllocateMemory(size * sizeof(char));
    if (!buf) {
        platformLogMessage(
            LOG_LEVEL_ERROR,
            "[ERROR]: Failed to allocate memory while trying to log a message");
        return;
    }

    // Write the log_level_string
    // At max 9 char are there + 1 for null so 10
    snprintf(buf, 10, "%s", log_level_strings[level]);

    va_start(args, msg);
    // NOTE: Write after log_level_string and also override the null character
    // by previous snprintf
    vsnprintf(
        (buf + ((level == LOG_LEVEL_WARN || level == LOG_LEVEL_INFO) ? 8 : 9)),
        size, msg, args);
    va_end(args);

#ifdef SPLATFORM_LINUX
    platformLogMessage(level, buf);
    platformDeallocateMemory(buf);
#else
    printf("%s\n", buf);
    free(buf);
#endif
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
