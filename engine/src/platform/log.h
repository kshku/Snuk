#pragma once

#include <stdarg.h>

#include "core/logger.h"
#include "defines.h"

void platformLogMessage(LogLevel level, const char *msg, va_list args,
                        const char *prefix, ...);
