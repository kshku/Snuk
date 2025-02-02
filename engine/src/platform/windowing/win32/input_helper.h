#pragma once

#include "defines.h"

#ifdef SPLATFORM_WINDOWING_WIN32
    #include "input/keyboard/keycode.h"
    #include "input/keyboard/scancode.h"

Scancode scan1MakeToScancode(u8 scancode, b8 is_extended);

Keycode virtualKeyCodeToKeycode(u16 virtual_keycode, u8 scancode,
                                b8 is_extended);

void syncKeymodsState(void);

void updateKeymodsState(u16 virtual_keycode, b8 pressed);

u32 getKeymods(void);

#endif
