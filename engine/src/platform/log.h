#pragma once

#include <stdarg.h>

#include "core/logger.h"
#include "defines.h"

void platformLogMessage(LogLevel level, const c8 *msg, va_list args,
                        const c8 *prefix, ...);
