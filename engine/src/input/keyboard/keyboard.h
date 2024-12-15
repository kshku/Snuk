#pragma once

#include "defines.h"
#include "keycode.h"
#include "scancode.h"

typedef struct KeyboardState {
        // b8 keycodes[KEYCODE_MAX_KEYCODE];
        b8 scancodes[SCANCODE_MAX_SCANCODE];
        Keymod mod;
} KeyboardState;

SAPI Keycode scancodeToKeycode(Scancode sc, Keymod mod);

SAPI Scancode keycodeToScancode(Keycode kc);
