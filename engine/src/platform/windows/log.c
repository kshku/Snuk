#include "../log.h"

#ifdef SPLATFORM_WINDOWS

    #include <stdio.h>
    #include <wchar.h>
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
    if (!failed && !GetConsoleMode(handle, &modes)) return false;

    modes |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING
           | DISABLE_NEWLINE_AUTO_RETURN;
    if (!failed && !SetConsoleMode(handle, modes)) return false;

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
 * @param msg The message to write
 */
void platformLogMessage(LogLevel level, const char *msg) {
    static const char *colors[6] = {
        "1;41", "1;31", "0;33",  // Fatal, Error, Warn
        "0;32", "0;34", "0;37"};  // Info, Debug, Trace

    // Track whether VT process is enabled for handle
    // 0 -> STD_OUTPUT_HANDLE, 1 -> STD_ERROR_HANDLE
    static b8 vt_enabled[2] = {false, false};

    // true = 1 and false = 0
    u8 error = level < LOG_LEVEL_WARN;
    if (!vt_enabled[error]) {  // VT processing is not enabled
        vt_enabled[error] =
            enableVTProcessing(error ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
    }

    if (error) {  // Fatal and Error
        if (!vt_enabled[error]) fprintf(stderr, "%s\n", msg);
        else fprintf(stderr, "\x1b[%sm%s\x1b[0m\n", colors[level], msg);
        fflush(NULL);
    } else {
        if (!vt_enabled[error]) fprintf(stdout, "%s\n", msg);
        else fprintf(stdout, "\x1b[%sm%s\x1b[0m\n", colors[level], msg);
    }
}

#endif
