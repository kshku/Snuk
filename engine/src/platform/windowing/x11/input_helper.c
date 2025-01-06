#include "input_helper.h"

#if defined(SPLATFORM_WINDOWING_X11_XCB) \
    || defined(SPLATFORM_WINDOWING_X11_XLIB)

    #include <X11/keysym.h>

    #include "core/sstring.h"

static const struct {
        c8 *name;
        Scancode code;
} name_code_map[] = {
    // Look at the xev output as well as the
    // /usr/share/X11/xkb/keycodes/evdev
    {"TLDE",            SCANCODE_GRAVE},
    {"AE01",                SCANCODE_1},
    {"AE02",                SCANCODE_2},
    {"AE03",                SCANCODE_3},
    {"AE04",                SCANCODE_4},
    {"AE05",                SCANCODE_5},
    {"AE06",                SCANCODE_6},
    {"AE07",                SCANCODE_7},
    {"AE08",                SCANCODE_8},
    {"AE09",                SCANCODE_9},
    {"AE10",                SCANCODE_0},
    {"AE11",            SCANCODE_MINUS},
    {"AE12",           SCANCODE_EQUALS},
    {"BKSP",        SCANCODE_BACKSPACE},

    { "TAB",              SCANCODE_TAB},
    {"AD01",                SCANCODE_Q},
    {"AD02",                SCANCODE_W},
    {"AD03",                SCANCODE_E},
    {"AD04",                SCANCODE_R},
    {"AD05",                SCANCODE_T},
    {"AD06",                SCANCODE_Y},
    {"AD07",                SCANCODE_U},
    {"AD08",                SCANCODE_I},
    {"AD09",                SCANCODE_O},
    {"AD10",                SCANCODE_P},
    {"AD11",     SCANCODE_LEFT_BRACKET},
    {"AD12",    SCANCODE_RIGHT_BRACKET},
    {"BKSL",        SCANCODE_BACKSLASH},
    {"RTRN",            SCANCODE_ENTER},

    {"CAPS",        SCANCODE_CAPS_LOCK},
    {"AC01",                SCANCODE_A},
    {"AC02",                SCANCODE_S},
    {"AC03",                SCANCODE_D},
    {"AC04",                SCANCODE_F},
    {"AC05",                SCANCODE_G},
    {"AC06",                SCANCODE_H},
    {"AC07",                SCANCODE_J},
    {"AC08",                SCANCODE_K},
    {"AC09",                SCANCODE_L},
    {"AC10",        SCANCODE_SEMICOLON},
    {"AC11",       SCANCODE_APOSTROPHE},
    {"AC12",        SCANCODE_BACKSLASH},

    {"LFSH",       SCANCODE_LEFT_SHIFT},
    {"LSGT", SCANCODE_NON_US_BACKSLASH},
    {"AB01",                SCANCODE_Z},
    {"AB02",                SCANCODE_X},
    {"AB03",                SCANCODE_C},
    {"AB04",                SCANCODE_V},
    {"AB05",                SCANCODE_B},
    {"AB06",                SCANCODE_N},
    {"AB07",                SCANCODE_M},
    {"AB08",            SCANCODE_COMMA},
    {"AB09",           SCANCODE_PERIOD},
    {"AB10",            SCANCODE_SLASH},
    {"RTSH",      SCANCODE_RIGHT_SHIFT},

    {"LCTL",     SCANCODE_LEFT_CONTROL},
    {"LWIN",         SCANCODE_LEFT_GUI},
    {"LALT",         SCANCODE_LEFT_ALT},
    {"SPCE",         SCANCODE_SPACEBAR},
    {"RALT",        SCANCODE_RIGHT_ALT},
    {"ALGR",        SCANCODE_RIGHT_ALT},
    {"RWIN",        SCANCODE_RIGHT_GUI},
    {"COMP",             SCANCODE_MENU},
    {"MENU",             SCANCODE_MENU},
    {"RCTL",    SCANCODE_RIGHT_CONTROL},

    { "ESC",           SCANCODE_ESCAPE},
    {"FK01",               SCANCODE_F1},
    {"FK02",               SCANCODE_F2},
    {"FK03",               SCANCODE_F3},
    {"FK04",               SCANCODE_F4},
    {"FK05",               SCANCODE_F5},
    {"FK06",               SCANCODE_F6},
    {"FK07",               SCANCODE_F7},
    {"FK08",               SCANCODE_F8},
    {"FK09",               SCANCODE_F9},
    {"FK10",              SCANCODE_F10},
    {"FK11",              SCANCODE_F11},
    {"FK12",              SCANCODE_F12},

    {"PRSC",     SCANCODE_PRINT_SCREEN},
    {"SCLK",      SCANCODE_SCROLL_LOCK},
    {"PAUS",            SCANCODE_PAUSE},

    { "INS",           SCANCODE_INSERT},
    {"HOME",             SCANCODE_HOME},
    {"PGUP",          SCANCODE_PAGE_UP},
    {"DELE",           SCANCODE_DELETE},
    { "END",              SCANCODE_END},
    {"PGDN",        SCANCODE_PAGE_DOWN},

    {  "UP",         SCANCODE_UP_ARROW},
    {"LEFT",       SCANCODE_LEFT_ARROW},
    {"DOWN",       SCANCODE_DOWN_ARROW},
    {"RGHT",      SCANCODE_RIGHT_ARROW},

    {"NMLK",         SCANCODE_NUM_LOCK},
    {"KPDV",    SCANCODE_KEYPAD_DIVIDE},
    {"KPMU",  SCANCODE_KEYPAD_MULTIPLY},
    {"KPSU",  SCANCODE_KEYPAD_SUBTRACT},

    { "KP7",         SCANCODE_KEYPAD_7},
    { "KP8",         SCANCODE_KEYPAD_8},
    { "KP9",         SCANCODE_KEYPAD_9},
    {"KPAD",       SCANCODE_KEYPAD_ADD},

    { "KP4",         SCANCODE_KEYPAD_4},
    { "KP5",         SCANCODE_KEYPAD_5},
    { "KP6",         SCANCODE_KEYPAD_6},

    { "KP1",         SCANCODE_KEYPAD_1},
    { "KP2",         SCANCODE_KEYPAD_2},
    { "KP3",         SCANCODE_KEYPAD_3},
    {"KPEN",     SCANCODE_KEYPAD_ENTER},

    { "KP0",         SCANCODE_KEYPAD_0},
    {"KPDL",    SCANCODE_KEYPAD_PERIOD},
    {"KPEQ",    SCANCODE_KEYPAD_EQUALS},

    {"FK13",              SCANCODE_F13},
    {"FK14",              SCANCODE_F14},
    {"FK15",              SCANCODE_F15},
    {"FK16",              SCANCODE_F16},
    {"FK17",              SCANCODE_F17},
    {"FK18",              SCANCODE_F18},
    {"FK19",              SCANCODE_F19},
    {"FK20",              SCANCODE_F20},
    {"FK21",              SCANCODE_F21},
    {"FK22",              SCANCODE_F22},
    {"FK23",              SCANCODE_F23},
    {"FK24",              SCANCODE_F24},
    // TODO: Add the other codes
    // NOTE: Have already added most of the commonly used ones
};

    #define NameCodeMapLength sizeof(name_code_map) / sizeof(name_code_map[0])

/**
 * @brief Get the Scancode from the symbolic keynames of X (Xkb's database).
 *
 * @param name Symbolic name of the key
 * @param len The length of the symoblic name
 *
 * @return Returns the corresponding Scancode for the given key name from the
 * name_code_map
 */
Scancode getScancodeForXKeyName(c8 *name) {
    for (u32 i = 0; i < NameCodeMapLength; ++i)
        if (sStringEqualC8(name, name_code_map[i].name, 4))
            return name_code_map[i].code;
    return SCANCODE_NONE;
}

/**
 * @brief Map the X's keycodes to the Scancodes.
 *
 * X KeyCodes will be mapped in in given map parameter. It's lenght should be
 * 256 since KeyCode is typedefed as unsigned char (At max it can only hold
 * value 255).
 *
 * Those parameters are returned by XkbGetNames or similar functions.
 *
 * @param map For each keycode Scancode will be mapped
 * @param key_names The 4-byte symbolic names from the xkb
 * @param key_alisa_names Alias names from the xkb
 * @param num_key_aliases Number of key aliases
 * @param min_key_code Minimum KeyCode
 * @param max_key_code Maximum KeyCode
 */
void mapXKeyCodesToScancodes(mapFunctionParams params, Scancode *map) {
    for (u32 keycode = params.min_key_code; keycode <= params.max_key_code;
         ++keycode) {
        u32 key_name_index = params.key_names_start_from_min_key_code
                               ? keycode - params.min_key_code
                               : keycode;
        map[keycode] =
            getScancodeForXKeyName(params.key_names[key_name_index].name);

        if (map[keycode] != SCANCODE_NONE) continue;

        // Look at the aliases if not found
        for (u32 i = 0; i < params.num_key_aliases; ++i) {
            if (map[keycode] != SCANCODE_NONE) break;

            if (!sStringEqualC8(params.key_aliases[i].real,
                                params.key_names[key_name_index].name, 4))
                continue;

            map[keycode] = getScancodeForXKeyName(params.key_aliases[i].alias);
        }
    }
}

/**
 * @brief Map the X's KeySym to Keycode (virtual keycodes provided by the
 * engine).
 *
 * @param sym The KeySym
 */
Keycode XKeySymToKeycode(u64 sym) {
    switch (sym) {
        case XK_BackSpace:
        case XK_Tab:
        case XK_Linefeed:
        case XK_Clear:
        case XK_Return:
        case XK_Escape:
            return (Keycode)(sym & (~0xff00));

        case XK_Pause:
            return KEYCODE_PAUSE;
        case XK_Scroll_Lock:
            return KEYCODE_SCROLL_LOCK;
        case XK_Sys_Req:
            return KEYCODE_SYSREQ;
        case XK_Delete:
            return KEYCODE_DELETE;

        case XK_Home:
            return KEYCODE_HOME;
        case XK_Left:
            return KEYCODE_LEFT_ARROW;
        case XK_Up:
            return KEYCODE_UP_ARROW;
        case XK_Right:
            return KEYCODE_RIGHT_ARROW;
        case XK_Down:
            return KEYCODE_DOWN_ARROW;
        case XK_Prior:
            return KEYCODE_PAGE_UP;
        case XK_Next:
            return KEYCODE_PAGE_DOWN;
        case XK_End:
            return KEYCODE_END;
        case XK_Begin:
            return KEYCODE_BEGIN;

        case XK_Select:
            return KEYCODE_SELECT;
        case XK_Print:
            return KEYCODE_PRINT_SCREEN;
        case XK_Execute:
            return KEYCODE_EXECUTE;
        case XK_Insert:
            return KEYCODE_INSERT;
        case XK_Undo:
            return KEYCODE_UNDO;
        case XK_Redo:
            return KEYCODE_REDO;
        case XK_Menu:
            return KEYCODE_MENU;
        case XK_Find:
            return KEYCODE_FIND;
        case XK_Cancel:
            return KEYCODE_CANCEL;
        case XK_Help:
            return KEYCODE_HELP;
        case XK_Break:
            return KEYCODE_BREAK;
        case XK_Num_Lock:
            return KEYCODE_NUM_LOCK;

        case XK_KP_Space:
            return KEYCODE_KEYPAD_SPACE;
        case XK_KP_Tab:
            return KEYCODE_KEYPAD_TAB;
        case XK_KP_Enter:
            return KEYCODE_KEYPAD_ENTER;
            // case XK_KP_F1:
            //     /* XK_KP_F1 -> PF1, KP_A, ... */
            //     return KEYCODE_KEYPAD_A;
            // case XK_KP_F2:
            //     return KEYCODE_KEYPAD_B;
            // case XK_KP_F3:
            //     return KEYCODE_KEYPAD_C;
            // case XK_KP_F4:
            //     return KEYCODE_KEYPAD_D;

        case XK_KP_Home:
            return KEYCODE_KEYPAD_HOME;
        case XK_KP_Left:
            return KEYCODE_KEYPAD_LEFT_ARROW;
        case XK_KP_Up:
            return KEYCODE_KEYPAD_UP_ARROW;
        case XK_KP_Right:
            return KEYCODE_KEYPAD_RIGHT_ARROW;
        case XK_KP_Down:
            return KEYCODE_KEYPAD_DOWN_ARROW;
        case XK_KP_Prior:
            return KEYCODE_KEYPAD_PAGE_UP;
        case XK_KP_Next:
            return KEYCODE_KEYPAD_PAGE_DOWN;
        case XK_KP_End:
            return KEYCODE_KEYPAD_END;
        case XK_KP_Begin:
            return KEYCODE_KEYPAD_BEGIN;
        case XK_KP_Insert:
            return KEYCODE_KEYPAD_INSERT;
        case XK_KP_Delete:
            return KEYCODE_KEYPAD_DELETE;
        case XK_KP_Equal:
            return KEYCODE_KEYPAD_EQUALS;
        case XK_KP_Multiply:
            return KEYCODE_KEYPAD_MULTIPLY;
        case XK_KP_Add:
            return KEYCODE_KEYPAD_ADD;
        case XK_KP_Separator:
            return KEYCODE_KEYPAD_COMMA;
        case XK_KP_Subtract:
            return KEYCODE_KEYPAD_SUBTRACT;
        case XK_KP_Decimal:
            return KEYCODE_KEYPAD_DECIMAL;
        case XK_KP_Divide:
            return KEYCODE_KEYPAD_DIVIDE;
        case XK_KP_0:
            return KEYCODE_KEYPAD_0;

        case XK_Shift_L:
            return KEYCODE_LEFT_SHIFT;
        case XK_Shift_R:
            return KEYCODE_RIGHT_SHIFT;
        case XK_Control_L:
            return KEYCODE_LEFT_CONTROL;
        case XK_Control_R:
            return KEYCODE_RIGHT_CONTROL;
        case XK_Caps_Lock:
            return KEYCODE_CAPS_LOCK;

        case XK_Alt_L:
            return KEYCODE_LEFT_ALT;
        case XK_Alt_R:
            return KEYCODE_RIGHT_ALT;
        case XK_Super_L:
            return KEYCODE_LEFT_GUI;
        case XK_Super_R:
            return KEYCODE_RIGHT_GUI;

        default:
            break;
    }

    if (sym >= XK_KP_1 && sym <= XK_KP_9)
        return (Keycode)(KEYCODE_KEYPAD_1 + (sym - XK_KP_1));

    if (sym >= XK_F1 && sym <= XK_F12)
        return (Keycode)(KEYCODE_F1 + (sym - XK_F1));

    if (sym >= XK_F13 && sym <= XK_F24)
        return (Keycode)(KEYCODE_F13 + (sym - XK_F13));

    if (sym >= XK_F25 && sym <= XK_F35)
        return (Keycode)(KEYCODE_F25 + (sym - XK_F25));

    if (sym >= XK_space && sym <= XK_asciitilde) return (Keycode)sym;

    return KEYCODE_NONE;
}

#endif
