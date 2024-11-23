#pragma once

#include "application_types.h"
#include "defines.h"

#ifdef SPLATFORM_OS_LINUX
// TODO: to decide between
// #define SPLATFORM_WINDOWING_X11_XLIB
    #define SPLATFORM_WINDOWING_X11_XCB
#endif

#ifdef SPLATFORM_OS_WINDOWS
    #define SPLATFORM_WINDOWING_WIN32
#endif

b8 initializePlatformWindowing(MainWindowConfig *config, u64 *size,
                               void *state);

void shutdownPlatformWindowing(void *state);

b8 platformWindowPumpMessages(void);

b8 platformWindowCreate();

void platformWindowDestroy();

b8 platformSetWindowVisible(b8 visible);

b8 platformSetWindowTitle(const char *title);

b8 platformGetWindowTitle(char *title, u64 size);
