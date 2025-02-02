#pragma once

#include "application_types.h"
#include "defines.h"

b8 initializePlatformWindowing(MainWindowConfig *config, u64 *size,
                               void *state);

void shutdownPlatformWindowing(void);

b8 platformWindowPumpMessages(void);

b8 platformWindowCreate();

void platformWindowDestroy();

b8 platformSetWindowVisible(b8 visible);

b8 platformSetWindowTitle(const char *title);

char *platformGetWindowTitle(void);
