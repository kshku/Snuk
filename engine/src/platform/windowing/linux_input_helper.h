#pragma once

#include "../window.h"

#if defined(SPLATFORM_WINDOWING_X11_XLIB)  \
    | defined(SPLATFORM_WINDOWING_X11_XCB) \
    | defined(SPLATFORM_WINDOWING_WAYLAND)
    #include <xkbcommon/xkbcommon.h>

    #include "defines.h"
    #include "input/keyboard/keycode.h"
    #include "input/keyboard/scancode.h"

Scancode getScancodeFromLinuxKeycode(u16 linux_keycode);

Keycode getKeycodeFromKeySym(u32 sym);

u32 getKeymodsFromXKBCommon(struct xkb_keymap *xkb_keymap,
                            struct xkb_state *xkb_state);

#endif
