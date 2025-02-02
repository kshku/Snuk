#pragma once

#include "defines.h"

#ifdef SPLATFORM_WINDOWING_X11_XLIB
    #include <X11/XKBlib.h>

void syncKeymodsState(Display *display);

void updateKeymodsState(u64 keysym, b8 pressed);

u32 getKeymods(void);

#endif
