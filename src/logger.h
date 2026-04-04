#pragma once

#include "defines.h"

#include <snlogger/log_level.h>

void snuk_logger_init(void);
void snuk_logger_deinit(void);

void snuk_log_msg(snLogLevel level, const char *file, const char *function, long line, const char *format_string, ...);

#define log_trace(msg, ...) snuk_log_msg(SN_LOG_LEVEL_TRACE, __FILE__, __func__, __LINE__, msg, ##__VA_ARGS__)
#define log_debug(msg, ...) snuk_log_msg(SN_LOG_LEVEL_DEBUG, __FILE__, __func__, __LINE__, msg, ##__VA_ARGS__)
#define log_info(msg, ...) snuk_log_msg(SN_LOG_LEVEL_INFO, __FILE__, __func__, __LINE__, msg, ##__VA_ARGS__)
#define log_warn(msg, ...) snuk_log_msg(SN_LOG_LEVEL_WARN, __FILE__, __func__, __LINE__, msg, ##__VA_ARGS__)
#define log_error(msg, ...) snuk_log_msg(SN_LOG_LEVEL_ERROR, __FILE__, __func__, __LINE__, msg, ##__VA_ARGS__)
#define log_fatal(msg, ...) snuk_log_msg(SN_LOG_LEVEL_FATAL, __FILE__, __func__, __LINE__, msg, ##__VA_ARGS__)
