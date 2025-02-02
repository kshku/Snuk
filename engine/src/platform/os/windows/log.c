#include "platform/log.h"

#ifdef SPLATFORM_OS_WINDOWS

    #include <stdio.h>
    #include <windows.h>

/**
 * @brief Helper function to enable VT Processing.
 *
 * https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
 *
 * @param handle_type Either STD_ERROR_HANDLE or STD_OUTPUT_HANDLE
 *
 * @return Returns true if VT processing is enbaled successfully, else false.
 */
b8 enableVTProcessing(DWORD handle_type) {
    HANDLE handle = GetStdHandle(handle_type);
    if (handle == INVALID_HANDLE_VALUE) return false;

    DWORD modes = 0;
    if (!GetConsoleMode(handle, &modes)) return false;

    modes |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING
           | DISABLE_NEWLINE_AUTO_RETURN;
    if (!SetConsoleMode(handle, modes)) return false;

    return true;
}

/**
 * @brief Logging helper implementation of Windows.
 *
 * This function uses the Virtual Terminal(VT) Processing for writing colored
 * ouput using Control Sequence Introducer (Same as the ANSI escape sequences).
 * If level was fatal or error, writes to STD_ERROR_HANDLE else writes to
 * STD_OUTPUT_HANDLE. If VT Processing couldn't be enabled, then writes to
 * stderr if fatal or error, else to stdout, without any escape sequences.
 *
 * https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
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

    // Track whether VT process is enabled for handle
    // 0 -> STD_OUTPUT_HANDLE, 1 -> STD_ERROR_HANDLE
    static b8 vt_enabled[2] = {false, false};

    // true = 1 and false = 0
    // Using u8 just because it is used to access the elements of vt_enabled.
    u8 error = level < LOG_LEVEL_WARN;  // Fatal or Error is error
    FILE *out_file = error ? stderr : stdout;

    // TODO: I don't think we need to try again since the handles will be
    // TODO: pointing to the same utill the app terminates. So call this only
    // TODO: once.
    // If previously couldn't able to turn on VT processing then will try
    // again whenever we call this function
    if (!vt_enabled[error])  // VT processing is not enabled, try to turn on
        vt_enabled[error] =
            enableVTProcessing(error ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);

    // If VT processing enabled, we can print color
    if (vt_enabled[error]) fprintf(out_file, "\x1b[%sm", colors[level]);

    va_list parg;
    va_start(parg, prefix);
    vfprintf(out_file, prefix, parg);
    va_end(parg);

    vfprintf(out_file, msg, args);

    // If VT processing enabled, reset the color after printing the message
    if (vt_enabled[error]) fprintf(out_file, "\x1b[0m");

    fprintf(out_file, "\n");

    if (error) fflush(NULL);
}

#endif
