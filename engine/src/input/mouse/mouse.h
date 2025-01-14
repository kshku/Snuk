#pragma once

#include "../keyboard/keycode.h"
#include "defines.h"

#define DEFINE_BUTTON(key, value) CONCAT(BUTTON_, key) = value
#define DEFINE_SCROLL(key, value) CONCAT(SCROLL_, key) = value

typedef enum Button {
    DEFINE_BUTTON(NONE, 0x00),

    DEFINE_BUTTON(LEFT, 0x01),
    DEFINE_BUTTON(MIDDLE, 0x02),
    DEFINE_BUTTON(RIGHT, 0x03),

    BUTTON_MAX_BUTTON
} Button;

// TODO: Scroll direction and scroll axis
// Converting and renaming 'Scroll'
typedef enum Scroll {
    DEFINE_SCROLL(NONE, 0x00),

    DEFINE_SCROLL(UP, 0x01),  // To the top of page
    DEFINE_SCROLL(DOWN, 0x02),  // To the bottom of page
    DEFINE_SCROLL(LEFT, 0x03),  // To the left of page
    DEFINE_SCROLL(RIGHT, 0x04),  // To the right of page

    SCROLL_MAX_SCROLL
} Scroll;

typedef struct ButtonState {
        b8 buttons[BUTTON_MAX_BUTTON];
        u32 x, y;
} ButtonState;

typedef struct ScrollState {
        b8 scrolls[SCROLL_MAX_SCROLL];
        u32 x, y;
} ScrollState;

// typedef struct PointerState {
//         u32 x, y;
// } PointerState;

typedef struct MouseState {
        ButtonState button_state;
        ScrollState scroll_state;
        // PointerState pointer_state;
        Keymod keymod;
} MouseState;

// typedef enum ScrollAxis {} ScrollAxis;

// void getScrollAxisAndValue()
