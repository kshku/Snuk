#pragma once

#include "defines.h"
#include "keycode.h"
#include "scancode.h"

typedef struct KeyboardState {
        // b8 keycodes[KEYCODE_MAX_KEYCODE];
        b8 scancodes[SCANCODE_MAX_SCANCODE];
        KeyMod mod;
} KeyboardState;

SAPI KeyCode scanCodeToKeyCode(ScanCode sc, KeyMod mod);

SAPI ScanCode keyCodeToScanCode(KeyCode kc);
