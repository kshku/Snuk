#pragma once

#include "defines.h"
#include "keycode.h"
#include "scancode.h"

typedef struct Key {
        ScanCode scancode;
        KeyCode keycode;
} Key;
