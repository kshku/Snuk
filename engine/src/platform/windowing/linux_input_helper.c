#include "linux_input_helper.h"

#if defined(SPLATFORM_WINDOWING_X11_XLIB)  \
    | defined(SPLATFORM_WINDOWING_X11_XCB) \
    | defined(SPLATFORM_WINDOWING_WAYLAND)

    #include <linux/input-event-codes.h>

    #include "core/logger.h"

static Scancode linux_keycode_scancode_map[KEY_CNT] = {
    [KEY_ESC] = SCANCODE_ESCAPE,
    [KEY_1] = SCANCODE_1,
    [KEY_2] = SCANCODE_2,
    [KEY_3] = SCANCODE_3,
    [KEY_4] = SCANCODE_4,
    [KEY_5] = SCANCODE_5,
    [KEY_6] = SCANCODE_6,
    [KEY_7] = SCANCODE_7,
    [KEY_8] = SCANCODE_8,
    [KEY_9] = SCANCODE_9,
    [KEY_0] = SCANCODE_0,
    [KEY_MINUS] = SCANCODE_MINUS,
    [KEY_EQUAL] = SCANCODE_EQUALS,
    [KEY_BACKSPACE] = SCANCODE_BACKSPACE,
    [KEY_TAB] = SCANCODE_TAB,
    [KEY_Q] = SCANCODE_Q,
    [KEY_W] = SCANCODE_W,
    [KEY_E] = SCANCODE_E,
    [KEY_R] = SCANCODE_R,
    [KEY_T] = SCANCODE_T,
    [KEY_Y] = SCANCODE_Y,
    [KEY_U] = SCANCODE_U,
    [KEY_I] = SCANCODE_I,
    [KEY_O] = SCANCODE_O,
    [KEY_P] = SCANCODE_P,
    [KEY_LEFTBRACE] = SCANCODE_LEFT_BRACKET,
    [KEY_RIGHTBRACE] = SCANCODE_RIGHT_BRACKET,
    [KEY_ENTER] = SCANCODE_ENTER,
    [KEY_LEFTCTRL] = SCANCODE_LEFT_CONTROL,
    [KEY_A] = SCANCODE_A,
    [KEY_S] = SCANCODE_S,
    [KEY_D] = SCANCODE_D,
    [KEY_F] = SCANCODE_F,
    [KEY_G] = SCANCODE_G,
    [KEY_H] = SCANCODE_H,
    [KEY_J] = SCANCODE_J,
    [KEY_K] = SCANCODE_K,
    [KEY_L] = SCANCODE_L,
    [KEY_SEMICOLON] = SCANCODE_SEMICOLON,
    [KEY_APOSTROPHE] = SCANCODE_APOSTROPHE,
    [KEY_GRAVE] = SCANCODE_GRAVE,
    [KEY_LEFTSHIFT] = SCANCODE_LEFT_SHIFT,
    [KEY_BACKSLASH] = SCANCODE_BACKSLASH,
    [KEY_Z] = SCANCODE_Z,
    [KEY_X] = SCANCODE_X,
    [KEY_C] = SCANCODE_C,
    [KEY_V] = SCANCODE_V,
    [KEY_B] = SCANCODE_B,
    [KEY_N] = SCANCODE_N,
    [KEY_M] = SCANCODE_M,
    [KEY_COMMA] = SCANCODE_COMMA,
    [KEY_DOT] = SCANCODE_PERIOD,
    [KEY_SLASH] = SCANCODE_SLASH,
    [KEY_RIGHTSHIFT] = SCANCODE_RIGHT_SHIFT,
    [KEY_KPASTERISK] = SCANCODE_KEYPAD_MULTIPLY,
    [KEY_LEFTALT] = SCANCODE_LEFT_ALT,
    [KEY_SPACE] = SCANCODE_SPACEBAR,
    [KEY_CAPSLOCK] = SCANCODE_CAPS_LOCK,
    [KEY_F1] = SCANCODE_F1,
    [KEY_F2] = SCANCODE_F2,
    [KEY_F3] = SCANCODE_F3,
    [KEY_F4] = SCANCODE_F4,
    [KEY_F5] = SCANCODE_F5,
    [KEY_F6] = SCANCODE_F6,
    [KEY_F7] = SCANCODE_F7,
    [KEY_F8] = SCANCODE_F8,
    [KEY_F9] = SCANCODE_F8,
    [KEY_F10] = SCANCODE_F10,
    [KEY_NUMLOCK] = SCANCODE_NUM_LOCK,
    [KEY_SCROLLLOCK] = SCANCODE_SCROLL_LOCK,
    [KEY_KP7] = SCANCODE_KEYPAD_7,
    [KEY_KP8] = SCANCODE_KEYPAD_8,
    [KEY_KP9] = SCANCODE_KEYPAD_9,
    [KEY_KPMINUS] = SCANCODE_KEYPAD_SUBTRACT,
    [KEY_KP4] = SCANCODE_KEYPAD_4,
    [KEY_KP5] = SCANCODE_KEYPAD_5,
    [KEY_KP6] = SCANCODE_KEYPAD_6,
    [KEY_KPPLUS] = SCANCODE_KEYPAD_ADD,
    [KEY_KP1] = SCANCODE_KEYPAD_1,
    [KEY_KP2] = SCANCODE_KEYPAD_2,
    [KEY_KP3] = SCANCODE_KEYPAD_3,
    [KEY_KP0] = SCANCODE_KEYPAD_0,
    [KEY_KPDOT] = SCANCODE_KEYPAD_PERIOD,
    [KEY_102ND] = SCANCODE_NON_US_BACKSLASH,
    [KEY_F11] = SCANCODE_F11,
    [KEY_F12] = SCANCODE_F12,
    // [KEY_RO] =
    // [KEY_KATAKANA] =
    // [KEY_HIRAGANA] =
    // [KEY_HENKAN] =
    // [KEY_KATAKANAHIRAGANA] =
    // [KEY_MUHENKAN] =
    // [KEY_KPJPCOMMA] = SCANCODE_KEYPAD_COMMA,
    [KEY_KPENTER] = SCANCODE_KEYPAD_ENTER,
    [KEY_RIGHTCTRL] = SCANCODE_RIGHT_CONTROL,
    [KEY_KPSLASH] = SCANCODE_KEYPAD_DIVIDE,
    [KEY_SYSRQ] = SCANCODE_SYSREQ,
    [KEY_RIGHTALT] = SCANCODE_RIGHT_ALT,
    // [KEY_LINEFEED] =
    [KEY_HOME] = SCANCODE_HOME,
    [KEY_UP] = SCANCODE_UP_ARROW,
    [KEY_PAGEUP] = SCANCODE_PAGE_UP,
    [KEY_LEFT] = SCANCODE_LEFT_ARROW,
    [KEY_RIGHT] = SCANCODE_RIGHT_ARROW,
    [KEY_END] = SCANCODE_END,
    [KEY_DOWN] = SCANCODE_DOWN_ARROW,
    [KEY_PAGEDOWN] = SCANCODE_PAGE_DOWN,
    [KEY_INSERT] = SCANCODE_INSERT,
    [KEY_DELETE] = SCANCODE_DELETE,
    // [KEY_MACRO] =
    [KEY_MUTE] = SCANCODE_MUTE,
    [KEY_VOLUMEDOWN] = SCANCODE_VOLUME_DOWN,
    [KEY_VOLUMEUP] = SCANCODE_VOLUME_UP,
    [KEY_POWER] = SCANCODE_POWER,
    [KEY_KPEQUAL] = SCANCODE_KEYPAD_EQUALS,
    [KEY_KPPLUSMINUS] = SCANCODE_KEYPAD_PLUS_MINUS,
    [KEY_PAUSE] = SCANCODE_PAUSE,
    // [KEY_SCALE] =
    [KEY_KPCOMMA] = SCANCODE_COMMA,
    // [KEY_HANGEUL] =
    // [KEY_HANJA] =
    // [KEY_YEN] =
    // [KEY_LEFTMETA] =
    // [KEY_RIGHTMETA] =
    // [KEY_COMPOSE] =
    [KEY_STOP] = SCANCODE_STOP,
    [KEY_AGAIN] = SCANCODE_AGAIN,
    // [KEY_PROPS] =
    [KEY_UNDO] = SCANCODE_UNDO,
    // [KEY_FRONT] =
    [KEY_COPY] = SCANCODE_COPY,
    // [KEY_OPEN] =
    [KEY_PASTE] = SCANCODE_PASTE,
    [KEY_FIND] = SCANCODE_FIND,
    [KEY_CUT] = SCANCODE_CUT,
    [KEY_HELP] = SCANCODE_HELP,
    [KEY_MENU] = SCANCODE_MENU,
    // [KEY_CALC] =
    // [KEY_SETUP] =
    // [KEY_SLEEP] =
    // [KEY_WAKEUP] =
    // [KEY_FILE] =
    // [KEY_SENDFILE] =
    // [KEY_DELETEFILE] =
    // [KEY_XFER] =
    // [KEY_PROG1] =
    // ...
    [KEY_F13] = SCANCODE_F13,
    [KEY_F14] = SCANCODE_F14,
    [KEY_F15] = SCANCODE_F15,
    [KEY_F16] = SCANCODE_F16,
    [KEY_F17] = SCANCODE_F17,
    [KEY_F18] = SCANCODE_F18,
    [KEY_F19] = SCANCODE_F19,
    [KEY_F20] = SCANCODE_F20,
    [KEY_F21] = SCANCODE_F21,
    [KEY_F22] = SCANCODE_F22,
    [KEY_F23] = SCANCODE_F23,
    [KEY_F24] = SCANCODE_F24,
    // ...
    // [KEY_ALL_APPLICATIONS] = SCANCODE_APPLICATION,
    // ...
    [KEY_PRINT] = SCANCODE_PRINT_SCREEN,
    // ...
    // [KEY_SEARCH]
    [KEY_ALTERASE] = SCANCODE_ALTERASE,
    [KEY_CANCEL] = SCANCODE_CANCEL,
    [KEY_SELECT] = SCANCODE_SELECT,
    [KEY_CLEAR] = SCANCODE_CLEAR};

/**
 * @brief Get the scancode for the linux keycode.
 *
 * @param linux_keycode The linux keycode
 *
 * @return Scancode for the given linux keycode
 */
Scancode getScancodeFromLinuxKeycode(u16 linux_keycode) {
    return linux_keycode_scancode_map[linux_keycode];
}

/**
 * @brief Get the keycode for the Keysym (virtual keycodes provided by the
 * engine).
 *
 * The keysym is may be from xkbcommon or from the X's protocol (as far as I
 * know both are having same keysym values).
 *
 * @param sym The KeySym
 *
 * @return Returns the keycode.
 */
Keycode getKeycodeFromKeySym(u32 sym) {
    switch (sym) {
        case XKB_KEY_BackSpace:
        case XKB_KEY_Tab:
        case XKB_KEY_Return:
        case XKB_KEY_Escape:
            return (Keycode)(sym & (~0xff00));

        case XKB_KEY_Clear:
            return KEYCODE_CLEAR;

        case XKB_KEY_Pause:
        case XKB_KEY_Break:
            return KEYCODE_PAUSE;
        case XKB_KEY_Scroll_Lock:
            return KEYCODE_SCROLL_LOCK;
        case XKB_KEY_Sys_Req:
            return KEYCODE_SYSREQ;
        case XKB_KEY_Delete:
            return KEYCODE_DELETE;

        case XKB_KEY_Home:
            return KEYCODE_HOME;
        case XKB_KEY_Left:
            return KEYCODE_LEFT_ARROW;
        case XKB_KEY_Up:
            return KEYCODE_UP_ARROW;
        case XKB_KEY_Right:
            return KEYCODE_RIGHT_ARROW;
        case XKB_KEY_Down:
            return KEYCODE_DOWN_ARROW;
        case XKB_KEY_Prior:
            return KEYCODE_PAGE_UP;
        case XKB_KEY_Next:
            return KEYCODE_PAGE_DOWN;
        case XKB_KEY_End:
            return KEYCODE_END;
        case XKB_KEY_Begin:
            return KEYCODE_BEGIN;

        case XKB_KEY_Select:
            return KEYCODE_SELECT;
        case XKB_KEY_Print:
            return KEYCODE_PRINT_SCREEN;
        case XKB_KEY_Execute:
            return KEYCODE_EXECUTE;
        case XKB_KEY_Insert:
            return KEYCODE_INSERT;
        case XKB_KEY_Undo:
            return KEYCODE_UNDO;
        case XKB_KEY_Redo:
            return KEYCODE_REDO;
        case XKB_KEY_Menu:
            return KEYCODE_MENU;
        case XKB_KEY_Find:
            return KEYCODE_FIND;
        case XKB_KEY_Cancel:
            return KEYCODE_CANCEL;
        case XKB_KEY_Help:
            return KEYCODE_HELP;
        case XKB_KEY_Num_Lock:
            return KEYCODE_NUM_LOCK;

        case XKB_KEY_KP_Space:
            return KEYCODE_KEYPAD_SPACE;
        case XKB_KEY_KP_Tab:
            return KEYCODE_KEYPAD_TAB;
        case XKB_KEY_KP_Enter:
            return KEYCODE_KEYPAD_ENTER;
            // case XKB_KEY_KP_F1:
            //     /* XKB_KEY_KP_F1 -> PF1, KP_A, ... */
            //     return KEYCODE_KEYPAD_A;
            // case XKB_KEY_KP_F2:
            //     return KEYCODE_KEYPAD_B;
            // case XKB_KEY_KP_F3:
            //     return KEYCODE_KEYPAD_C;
            // case XKB_KEY_KP_F4:
            //     return KEYCODE_KEYPAD_D;

        case XKB_KEY_KP_Home:
            return KEYCODE_KEYPAD_7;
        case XKB_KEY_KP_Left:
            return KEYCODE_KEYPAD_4;
        case XKB_KEY_KP_Up:
            return KEYCODE_KEYPAD_8;
        case XKB_KEY_KP_Right:
            return KEYCODE_KEYPAD_6;
        case XKB_KEY_KP_Down:
            return KEYCODE_KEYPAD_2;
        case XKB_KEY_KP_Prior:
            return KEYCODE_KEYPAD_9;
        case XKB_KEY_KP_Next:
            return KEYCODE_KEYPAD_3;
        case XKB_KEY_KP_End:
            return KEYCODE_KEYPAD_1;
            // In my pc pressing keypad 5 when numlock is off generates Begin
            // key in xev
        case XKB_KEY_KP_Begin:
            return KEYCODE_KEYPAD_5;
        case XKB_KEY_KP_Insert:
            return KEYCODE_KEYPAD_0;
        case XKB_KEY_KP_Delete:
            return KEYCODE_KEYPAD_PERIOD;
        case XKB_KEY_KP_Equal:
            return KEYCODE_KEYPAD_EQUALS;
        case XKB_KEY_KP_Multiply:
            return KEYCODE_KEYPAD_MULTIPLY;
        case XKB_KEY_KP_Add:
            return KEYCODE_KEYPAD_ADD;
        case XKB_KEY_KP_Separator:
            return KEYCODE_KEYPAD_COMMA;
        case XKB_KEY_KP_Subtract:
            return KEYCODE_KEYPAD_SUBTRACT;
        case XKB_KEY_KP_Decimal:
            // keysymdef.h says it is full stop
            return KEYCODE_KEYPAD_PERIOD;
        case XKB_KEY_KP_Divide:
            return KEYCODE_KEYPAD_DIVIDE;
        case XKB_KEY_KP_0:
            return KEYCODE_KEYPAD_0;

        case XKB_KEY_Shift_L:
            return KEYCODE_LEFT_SHIFT;
        case XKB_KEY_Shift_R:
            return KEYCODE_RIGHT_SHIFT;
        case XKB_KEY_Control_L:
            return KEYCODE_LEFT_CONTROL;
        case XKB_KEY_Control_R:
            return KEYCODE_RIGHT_CONTROL;
        case XKB_KEY_Caps_Lock:
            return KEYCODE_CAPS_LOCK;

        case XKB_KEY_Alt_L:
            return KEYCODE_LEFT_ALT;
        case XKB_KEY_Alt_R:
            return KEYCODE_RIGHT_ALT;
        case XKB_KEY_Super_L:
            return KEYCODE_LEFT_GUI;
        case XKB_KEY_Super_R:
            return KEYCODE_RIGHT_GUI;

        case XKB_KEY_space:
            return KEYCODE_SPACE;

        case XKB_KEY_exclam:
            return KEYCODE_1;

        case XKB_KEY_quotedbl:
        case XKB_KEY_apostrophe:
            return KEYCODE_APOSTROPHE;

        case XKB_KEY_ampersand:
            return KEYCODE_7;
        case XKB_KEY_parenleft:
            return KEYCODE_9;
        case XKB_KEY_parenright:
            return KEYCODE_0;
        case XKB_KEY_asterisk:
            return KEYCODE_8;

        case XKB_KEY_equal:
        case XKB_KEY_plus:
            return KEYCODE_EQUALS;

        case XKB_KEY_semicolon:
        case XKB_KEY_colon:
            return KEYCODE_SEMICOLON;

        case XKB_KEY_less:
            return KEYCODE_COMMA;
        case XKB_KEY_question:
            return KEYCODE_SLASH;
        case XKB_KEY_at:
            return KEYCODE_2;
        case XKB_KEY_asciicircum:
            return KEYCODE_6;

        case XKB_KEY_underscore:
            return KEYCODE_MINUS;

        case XKB_KEY_braceleft:
            return KEYCODE_LEFT_BRACKET;

        case XKB_KEY_bar:
            return KEYCODE_BACKSLASH;
        case XKB_KEY_braceright:
            return KEYCODE_RIGHT_BRACKET;

        case XKB_KEY_asciitilde:
        case XKB_KEY_grave:
            return KEYCODE_GRAVE;

        case XKB_KEY_Mode_switch:
            return KEYCODE_ALTGR;

        default:
            break;
    }

    if (sym >= XKB_KEY_KP_1 && sym <= XKB_KEY_KP_9)
        return (Keycode)(KEYCODE_KEYPAD_1 + (sym - XKB_KEY_KP_1));

    if (sym >= XKB_KEY_F1 && sym <= XKB_KEY_F12)
        return (Keycode)(KEYCODE_F1 + (sym - XKB_KEY_F1));

    if (sym >= XKB_KEY_F13 && sym <= XKB_KEY_F24)
        return (Keycode)(KEYCODE_F13 + (sym - XKB_KEY_F13));

    if (sym >= XKB_KEY_F25 && sym <= XKB_KEY_F35)
        return (Keycode)(KEYCODE_F25 + (sym - XKB_KEY_F25));

    if (sym >= XKB_KEY_numbersign && sym <= XKB_KEY_percent)
        return (Keycode)(KEYCODE_3 + (sym - XKB_KEY_numbersign));

    if (sym >= XKB_KEY_comma && sym <= XKB_KEY_9) return (Keycode)sym;

    if (sym >= XKB_KEY_A && sym <= XKB_KEY_bracketright) return (Keycode)sym;

    if (sym >= XKB_KEY_a && sym <= XKB_KEY_z)
        return (Keycode)(KEYCODE_A + (sym - XKB_KEY_a));

    return KEYCODE_NONE;
}

/**
 * @brief Get the modifiers (as in the Keymod) from xkb_state.
 *
 * @param xkb_state xkb state to use
 *
 * @return Returns the Keymod modifiers.
 */
u32 getKeymodsFromXKBCommon(struct xkb_keymap *xkb_keymap,
                            struct xkb_state *xkb_state) {
    UNUSED(xkb_keymap);
    // TODO: Using the below one (may be)
    // u32 n = xkb_keymap_num_mods(xkb_keymap);
    // for (u32 i = 0; i < n; ++i) {
    //     sDebug("%dth Mod mask name: '%s'", i,
    //            xkb_keymap_mod_get_name(xkb_keymap, i));
    // }

    u32 ret = 0;

    struct {
            char *name;
            Keymod mod;
    } index_map[] = {
        {XKB_MOD_NAME_SHIFT,       KEYMOD_SHIFT},
        { XKB_MOD_NAME_CAPS,   KEYMOD_CAPS_LOCK},
        { XKB_MOD_NAME_CTRL,     KEYMOD_CONTROL},
        {  XKB_MOD_NAME_ALT,         KEYMOD_ALT},
        {  XKB_MOD_NAME_NUM,    KEYMOD_NUM_LOCK},
        {            "Mod3",       KEYMOD_ALTGR},
        { XKB_MOD_NAME_LOGO,         KEYMOD_GUI},
        {            "Mod5", KEYMOD_SCROLL_LOCK}
    };

    u32 len = sizeof(index_map) / sizeof(index_map[0]);

    for (u32 i = 0; i < len; ++i) {
        i32 err = xkb_state_mod_name_is_active(xkb_state, index_map[i].name,
                                               XKB_STATE_MODS_EFFECTIVE);
        if (err == -1)
            sError("Couldn't find the modifier with name '%s'",
                   index_map[i].name);
        else ret |= err ? index_map[i].mod : 0;
    }

    return ret;
}

#endif
