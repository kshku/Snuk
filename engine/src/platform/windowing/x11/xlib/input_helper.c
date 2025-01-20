#include "input_helper.h"

#ifdef SPLATFORM_WINDOWING_X11_XLIB

    #include <X11/keysym.h>

    #include "core/memory/memory.h"
    #include "input/keyboard/keycode.h"

typedef enum XModMask {
    X_SHFIT_MASK = 0,
    X_LOCK_MASK = 1,
    X_CONTROL_MASK = 2,
    X_MOD_1_MASK = 3,
    X_MOD_2_MASK = 4,
    X_MOD_3_MASK = 5,
    X_MOD_4_MASK = 6,
    X_MOD_5_MASK = 7,
    X_BUTTON_1_MASK = 8,
    X_BUTTON_2_MASK = 9,
    X_BUTTON_3_MASK = 10,
    X_BUTTON_4_MASK = 11,
    X_BUTTON_5_MASK = 12,
    X_MOD_MASK_MAX
} XModMask;

// https://stackoverflow.com/questions/19376338/xcb-keyboard-button-masks-meaning
// Also xmodmap -pm
// And my memory of looking at some source code repos.
static const Keymod xmod_keymod_map[X_MOD_MASK_MAX] = {
    [X_SHFIT_MASK] = KEYMOD_SHIFT,     [X_LOCK_MASK] = KEYMOD_CAPS_LOCK,
    [X_CONTROL_MASK] = KEYMOD_CONTROL, [X_MOD_1_MASK] = KEYMOD_ALT,
    [X_MOD_2_MASK] = KEYMOD_NUM_LOCK,  [X_MOD_3_MASK] = KEYMOD_ALTGR,
    [X_MOD_4_MASK] = KEYMOD_GUI,       [X_MOD_5_MASK] = KEYMOD_SCROLL_LOCK,
    [X_BUTTON_1_MASK] = KEYMOD_NONE,   [X_BUTTON_2_MASK] = KEYMOD_NONE,
    [X_BUTTON_3_MASK] = KEYMOD_NONE,   [X_BUTTON_4_MASK] = KEYMOD_NONE,
    [X_BUTTON_5_MASK] = KEYMOD_NONE,
};

static u32 keymod_state;
    #define ADD_KEYMOD(keymod) (keymod_state |= keymod)
    #define REMOVE_KEYMOD(keymod) (keymod_state &= (~keymod))
    #define TOGGLE_KEYMOD(keymod) \
        (keymod_state & keymod ? REMOVE_KEYMOD(keymod) : ADD_KEYMOD(keymod))

/**
 * @brief Sync the modifiers state.
 *
 * Uses the XkbGetState function to get the state of the modifiers especially
 * state of the lock keys (caps lock, num lock, etc.)
 */
void syncKeymodsState(Display *display) {
    keymod_state = 0;

    XkbStatePtr xkb_state = sMalloc(sizeof(XkbStateRec));
    // TODO: Error handling
    XkbGetState(display, XkbUseCoreKbd, xkb_state);

    for (u32 mod = xkb_state->mods, i = 0; mod; mod >>= 1, ++i)
        if (mod & 1) ADD_KEYMOD(xmod_keymod_map[i]);

    sFree(xkb_state);
}

/**
 * @brief Update the Keymods state.
 *
 * @param keysym The keysym whose state changed
 * @param pressed state of the keysym (pressed or not)
 */
void updateKeymodsState(u64 keysym, b8 pressed) {
    Keymod mod = KEYMOD_NONE;
    Keymod lock = KEYMOD_NONE;
    switch (keysym) {
        case XK_Shift_L:
            mod = KEYMOD_LEFT_SHIFT;
            break;
        case XK_Shift_R:
            mod = KEYMOD_RIGHT_SHIFT;
            break;
        case XK_Control_L:
            mod = KEYMOD_LEFT_CONTROL;
            break;
        case XK_Control_R:
            mod = KEYMOD_RIGHT_CONTROL;
            break;
        case XK_Alt_L:
            mod = KEYMOD_LEFT_ALT;
            break;
        case XK_Alt_R:
            mod = KEYMOD_RIGHT_ALT;
            break;
        case XK_Super_L:
            mod = KEYMOD_LEFT_SUPER;
            break;
        case XK_Super_R:
            mod = KEYMOD_RIGHT_SUPER;
            break;
        case XK_Caps_Lock:
            lock = KEYMOD_CAPS_LOCK;
            break;
        case XK_Num_Lock:
            lock = KEYMOD_NUM_LOCK;
            break;
        case XK_Scroll_Lock:
            lock = KEYMOD_SCROLL_LOCK;
            break;
            // TODO: AltGr?
        default:
            mod = KEYMOD_NONE;
            lock = KEYMOD_NONE;
            break;
    }

    if (pressed) {
        ADD_KEYMOD(mod);
    } else {
        REMOVE_KEYMOD(mod);
        TOGGLE_KEYMOD(lock);
    }
}

/**
 * @brief Get the current modifiers state.
 *
 * @return The Keymods that are active.
 */
u32 getKeymods(void) {
    return keymod_state;
}

#endif
