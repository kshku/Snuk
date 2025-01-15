
#include "input_helper.h"

#ifdef SPLATFORM_WINDOWING_WIN32

    #include "Windows.h"
    #include "core/logger.h"

// The value of scancode at max may be 0xff and if key is extended, we or the
// scancode with 0x100. So the maximum size of this array is 0x1ff
// If we leave the size empty the size will be largest index we defined inside
// the array. And while accessing the map if we get index greater than size...
Scancode scan1make_to_scancode_map[0x1FF] = {
    [0x01E] = SCANCODE_A,
    [0x030] = SCANCODE_B,
    [0x02E] = SCANCODE_C,
    [0x020] = SCANCODE_D,
    [0x012] = SCANCODE_E,
    [0x021] = SCANCODE_F,
    [0x022] = SCANCODE_G,
    [0x023] = SCANCODE_H,
    [0x017] = SCANCODE_I,
    [0x024] = SCANCODE_J,
    [0x025] = SCANCODE_K,
    [0x026] = SCANCODE_L,
    [0x032] = SCANCODE_M,
    [0x031] = SCANCODE_N,
    [0x018] = SCANCODE_O,
    [0x019] = SCANCODE_P,
    [0x010] = SCANCODE_Q,
    [0x013] = SCANCODE_R,
    [0x01F] = SCANCODE_S,
    [0x014] = SCANCODE_T,
    [0x016] = SCANCODE_U,
    [0x02F] = SCANCODE_V,
    [0x011] = SCANCODE_W,
    [0x02D] = SCANCODE_X,
    [0x015] = SCANCODE_Y,
    [0x02C] = SCANCODE_Z,

    [0x002] = SCANCODE_1,
    [0x003] = SCANCODE_2,
    [0x004] = SCANCODE_3,
    [0x005] = SCANCODE_4,
    [0x006] = SCANCODE_5,
    [0x007] = SCANCODE_6,
    [0x008] = SCANCODE_7,
    [0x009] = SCANCODE_8,
    [0x00A] = SCANCODE_9,
    [0x00B] = SCANCODE_0,

    [0x01C] = SCANCODE_ENTER,
    [0x001] = SCANCODE_ESCAPE,
    [0x00E] = SCANCODE_DELETE,
    [0x00F] = SCANCODE_TAB,
    [0x039] = SCANCODE_SPACEBAR,
    [0x00C] = SCANCODE_MINUS,
    [0x00D] = SCANCODE_EQUALS,
    [0x01A] = SCANCODE_LEFT_BRACKET,
    [0x01B] = SCANCODE_RIGHT_BRACKET,
    [0x02B] = SCANCODE_BACKSLASH,
    // [0x02B] = SCANCODE_NON_US_HASH
    [0x027] = SCANCODE_SEMICOLON,
    [0x028] = SCANCODE_APOSTROPHE,
    [0x029] = SCANCODE_GRAVE,
    [0x033] = SCANCODE_COMMA,
    [0x034] = SCANCODE_PERIOD,
    [0x035] = SCANCODE_SLASH,
    [0x03A] = SCANCODE_CAPS_LOCK,

    [0x03B] = SCANCODE_F1,
    [0x03C] = SCANCODE_F2,
    [0x03D] = SCANCODE_F3,
    [0x03E] = SCANCODE_F4,
    [0x03F] = SCANCODE_F5,
    [0x040] = SCANCODE_F6,
    [0x041] = SCANCODE_F7,
    [0x042] = SCANCODE_F8,
    [0x043] = SCANCODE_F9,
    [0x044] = SCANCODE_F10,
    [0x057] = SCANCODE_F11,
    [0x058] = SCANCODE_F12,

    [0x137] = SCANCODE_PRINT_SCREEN,
    [0x054] = SCANCODE_SYSREQ,
    [0x046] = SCANCODE_SCROLL_LOCK,
    [0x045] = SCANCODE_PAUSE,
    // Ctrl + Pause = Break (0xE046), but there is no scancode for it in HID usb
    // documentation (0x07)
    // I don't know about 0xE11D45
    [0x146] = SCANCODE_PAUSE,
    [0x152] = SCANCODE_INSERT,
    [0x147] = SCANCODE_HOME,
    [0x149] = SCANCODE_PAGE_UP,
    [0x153] = SCANCODE_DELETE,
    [0x14F] = SCANCODE_END,
    [0x151] = SCANCODE_PAGE_DOWN,
    [0x14D] = SCANCODE_RIGHT_ARROW,
    [0x14B] = SCANCODE_LEFT_ARROW,
    [0x150] = SCANCODE_DOWN_ARROW,
    [0x148] = SCANCODE_UP_ARROW,

    // Document says for keypad Num Lock and Clear 0x0045 and 0xE045 as seen in
    // legacy keyboard messages. I checked what scancode is returned when key is
    // pressed using a hook to write this. It looks like those scan 1 make codes
    // with note 3(which says as seen in legacy keyboard messages) are the one
    // which are delivered.
    [0x145] = SCANCODE_NUM_LOCK,

    [0x135] = SCANCODE_KEYPAD_DIVIDE,
    [0x037] = SCANCODE_KEYPAD_MULTIPLY,
    [0x04A] = SCANCODE_KEYPAD_SUBTRACT,
    [0x04E] = SCANCODE_KEYPAD_ADD,
    [0x11C] = SCANCODE_KEYPAD_ENTER,

    [0x04F] = SCANCODE_KEYPAD_1,
    [0x050] = SCANCODE_KEYPAD_2,
    [0x051] = SCANCODE_KEYPAD_3,
    [0x04B] = SCANCODE_KEYPAD_4,
    [0x04C] = SCANCODE_KEYPAD_5,
    [0x04D] = SCANCODE_KEYPAD_6,
    [0x047] = SCANCODE_KEYPAD_7,
    [0x048] = SCANCODE_KEYPAD_8,
    [0x049] = SCANCODE_KEYPAD_9,
    [0x052] = SCANCODE_KEYPAD_0,

    [0x053] = SCANCODE_KEYPAD_PERIOD,

    [0x056] = SCANCODE_NON_US_BACKSLASH,
    [0x15D] = SCANCODE_APPLICATION,
    [0x15E] = SCANCODE_POWER,
    [0x059] = SCANCODE_KEYPAD_EQUALS,

    [0x064] = SCANCODE_F13,
    [0x065] = SCANCODE_F14,
    [0x066] = SCANCODE_F15,
    [0x067] = SCANCODE_F16,
    [0x068] = SCANCODE_F17,
    [0x069] = SCANCODE_F18,
    [0x06A] = SCANCODE_F19,
    [0x06B] = SCANCODE_F20,
    [0x06C] = SCANCODE_F21,
    [0x06D] = SCANCODE_F22,
    [0x06E] = SCANCODE_F23,
    [0x076] = SCANCODE_F24,

    [0x07E] = SCANCODE_KEYPAD_COMMA,

    [0x073] = SCANCODE_INTERNATIONAL_1,
    [0x070] = SCANCODE_INTERNATIONAL_2,
    [0x07D] = SCANCODE_INTERNATIONAL_3,
    [0x079] = SCANCODE_INTERNATIONAL_4,
    [0x07B] = SCANCODE_INTERNATIONAL_5,
    [0x05C] = SCANCODE_INTERNATIONAL_6,
    // I don't know which key to press for those, so if there is note 3 then
    // using it when there are multiple scan 1 make codes. Also the scan 1 make
    // code is not used for any other key then for that scan 1 make code too
    // assigning the same scancode (HID usb scancode).
    [0x072] = SCANCODE_LANG_1,
    [0x0F2] = SCANCODE_LANG_1,
    [0x071] = SCANCODE_LANG_2,
    [0x0F1] = SCANCODE_LANG_2,
    [0x078] = SCANCODE_LANG_3,
    [0x077] = SCANCODE_LANG_4,
    // Both F24 and LANG5 keys have same scan 1 make CODE.
    // [0x076] = SCANCODE_LANG_5,

    [0x01D] = SCANCODE_LEFT_CONTROL,
    [0x02A] = SCANCODE_LEFT_SHIFT,
    [0x038] = SCANCODE_LEFT_ALT,
    [0x15B] = SCANCODE_LEFT_GUI,

    [0x11D] = SCANCODE_RIGHT_CONTROL,
    [0x036] = SCANCODE_RIGHT_SHIFT,
    [0x138] = SCANCODE_RIGHT_ALT,
    [0x15C] = SCANCODE_RIGHT_GUI,
};

/**
 * @brief Convert the scan 1 make codes to the Scancode.
 *
 * As shown in the documentation 16-23 bits of lParam have the scancode(1 byte)
 * and the 24th bit represents whether the key is extended key.
 * Pass the 8 bit scancode and whether the key is extended (non-zero if
 * extended, zero if not) as the parameter.
 *
 * @param scancode 8 bit scancode
 * @param is_extended extended key or not
 *
 * @return Scancode for the corresponding scan 1 make code.
 *
 * @example
 * In the WindowProcedure function
 * Scancode sc = scan1MakeToScancode(LOBYTE(HIWORD(lParam)), (HIWORD(lParam) &
 * KF_EXTENDED));
 */
Scancode scan1MakeToScancode(u8 scancode, b8 is_extended) {
    return scan1make_to_scancode_map[scancode | (is_extended ? 0x100 : 0)];
}

/**
 * @brief Convert the windows' virtual keycode to Keycode.
 *
 * As shown in the documentation 16-23 bits of lParam have the scancode(1 byte)
 * and the 24th bit represents whether the key is extended key.
 * Pass the 8 bit scancode and whether the key is extended (non-zero if
 * extended, zero if not) as the parameter. For virtual keycode the wParam is
 * the virtual keycode in the windowProcedure function.
 *
 * @param virtual_keycode Virtual keycode to be translated
 * @param scancode 8 bit scancode
 * @param is_extended extended key or not
 *
 * @return The Keycode for correspoding virtual keycode
 *
 * @example
 * In the windowProcedure function
 * Keycode kc = windowsVirtualKeyCodeToKeycode(LOWORD(wParam),
 * LOBYTE(HIWORD(lParam)), (HIWORD(lParam) & KF_EXTENDED));
 */
Keycode virtualKeyCodeToKeycode(u16 virtual_keycode, u8 scancode,
                                b8 is_extended) {
    // TODO: Make thing right
    // Get whether right key or left key
    if (virtual_keycode >= VK_SHIFT && virtual_keycode <= VK_MENU)
        virtual_keycode = LOWORD(MapVirtualKey(
            (scancode | (is_extended ? 0xE000 : 0)), MAPVK_VSC_TO_VK_EX));

    switch (virtual_keycode) {
        case VK_CANCEL:
            return KEYCODE_CANCEL;
        case VK_BACK:
        case VK_TAB:
        case VK_RETURN:
        case VK_ESCAPE:
        case VK_SPACE:
            return (Keycode)virtual_keycode;

        case VK_CLEAR:
            return KEYCODE_CLEAR;
        case VK_PAUSE:
            return KEYCODE_PAUSE;
        case VK_CAPITAL:
            return KEYCODE_CAPS_LOCK;

        case VK_PRIOR:
            return KEYCODE_PAGE_UP;
        case VK_NEXT:
            return KEYCODE_PAGE_DOWN;
        case VK_END:
            return KEYCODE_END;
        case VK_HOME:
            return KEYCODE_HOME;

        case VK_LEFT:
            return KEYCODE_LEFT_ARROW;
        case VK_UP:
            return KEYCODE_UP_ARROW;
        case VK_RIGHT:
            return KEYCODE_RIGHT_ARROW;
        case VK_DOWN:
            return KEYCODE_DOWN_ARROW;

        case VK_SELECT:
            return KEYCODE_SELECT;
        // case VK_PRINT:
        // https://stackoverflow.com/questions/5815471/whats-the-purpose-of-vk-print
        case VK_EXECUTE:
            return KEYCODE_EXECUTE;
        case VK_SNAPSHOT:
            return KEYCODE_PRINT_SCREEN;
        case VK_INSERT:
            return KEYCODE_INSERT;
        case VK_DELETE:
            return KEYCODE_DELETE;
        case VK_HELP:
            return KEYCODE_HELP;

        case VK_LWIN:
            return KEYCODE_LEFT_GUI;
        case VK_RWIN:
            return KEYCODE_RIGHT_GUI;

        case VK_APPS:
            return KEYCODE_APPLICATION;
            // case VK_SLEEP:

        case VK_NUMPAD0:
            return KEYCODE_KEYPAD_0;

        // TODO: Verify the mappings
        // I am not sure about these mainly
        case VK_MULTIPLY:
            return KEYCODE_KEYPAD_MULTIPLY;
        case VK_ADD:
            return KEYCODE_KEYPAD_ADD;
        case VK_SEPARATOR:
            return KEYCODE_SEPARATOR;
        case VK_SUBTRACT:
            return KEYCODE_KEYPAD_SUBTRACT;
        case VK_DECIMAL:
            return KEYCODE_KEYPAD_DECIMAL;
        case VK_DIVIDE:
            return KEYCODE_KEYPAD_DIVIDE;

        case VK_NUMLOCK:
            return KEYCODE_NUM_LOCK;
        case VK_SCROLL:
            return KEYCODE_SCROLL_LOCK;

        case VK_LSHIFT:
            return KEYCODE_LEFT_SHIFT;
        case VK_RSHIFT:
            return KEYCODE_RIGHT_SHIFT;

        case VK_LCONTROL:
            return KEYCODE_LEFT_CONTROL;
        case VK_RCONTROL:
            return KEYCODE_RIGHT_CONTROL;

        case VK_LMENU:
            return KEYCODE_LEFT_ALT;
        case VK_RMENU:
            return KEYCODE_RIGHT_ALT;

        case VK_OEM_1:
            return KEYCODE_SEMICOLON;
        case VK_OEM_PLUS:
            return KEYCODE_EQUALS;
        case VK_OEM_COMMA:
            return KEYCODE_COMMA;
        case VK_OEM_MINUS:
            return KEYCODE_MINUS;
        case VK_OEM_PERIOD:
            return KEYCODE_PERIOD;
        case VK_OEM_2:
            return KEYCODE_SLASH;
        case VK_OEM_3:
            return KEYCODE_GRAVE;
        case VK_OEM_4:
            return KEYCODE_LEFT_BRACKET;
        case VK_OEM_5:
            return KEYCODE_BACKSLASH;
        case VK_OEM_6:
            return KEYCODE_RIGHT_BRACKET;
        case VK_OEM_7:
            return KEYCODE_APOSTROPHE;
            // case VK_OEM_8:
        case VK_OEM_102:
            return KEYCODE_NON_US_BACKSLASH;

        case VK_CRSEL:
            return KEYCODE_CRSEL;
        case VK_EXSEL:
            return KEYCODE_EXSEL;
            // case VK_EREOF:

        default:
            break;
    }

    if (virtual_keycode >= 0x30 && virtual_keycode <= 0x39)
        return (Keycode)virtual_keycode;

    if (virtual_keycode >= 0x41 && virtual_keycode <= 0x5A)
        return (Keycode)virtual_keycode;

    if (virtual_keycode >= VK_NUMPAD1 && virtual_keycode <= VK_NUMPAD9)
        return (Keycode)(SCANCODE_KEYPAD_1 + (virtual_keycode - VK_NUMPAD1));

    if (virtual_keycode >= VK_F1 && virtual_keycode <= VK_F12)
        return (Keycode)(KEYCODE_F1 + (virtual_keycode - VK_F1));

    if (virtual_keycode >= VK_F13 && virtual_keycode <= VK_F24)
        return (Keycode)(KEYCODE_F13 + (virtual_keycode - VK_F13));

    return KEYCODE_NONE;
}

// typedef enum KeymodIndex {
//     KEYMOD_INDEX_SHIFT = 0,
//     KEYMOD_SHIFT_CONTROL = 1,
//     KEYMOD_INDEX_ALT = 2,
//     KEYMOD_INDEX_GUI = 3,
//     KEYMOD_INDEX_NUM_LOCK = 4,
//     KEYMOD_INDEX_CAPS_LOCK = 5,
//     KEYMOD_INDEX_SCROLL_LOCK = 6,
//     KEYMOD_INDEX_ALTGR = 7
// } KeymodIndex;

static u32 keymod_state;
    #define ADD_KEYMOD(keymod) (keymod_state |= keymod)
    #define REMOVE_KEYMOD(keymod) (keymod_state &= (~keymod))
    #define TOGGLE_KEYMOD(keymod) \
        (keymod_state & keymod ? REMOVE_KEYMOD(keymod) : ADD_KEYMOD(keymod))

/**
 * @brief Sync the modifiers state.
 *
 * Uses the GetKeyState function to get the state of the modifiers especially
 * state of the lock keys (caps lock, num lock, etc.)
 */
void syncKeymodsState(void) {
    keymod_state = 0;
    if (GetKeyState(VK_SHIFT) & BITFLAG(15)) ADD_KEYMOD(KEYMOD_SHIFT);
    if (GetKeyState(VK_CONTROL) & BITFLAG(15)) ADD_KEYMOD(KEYMOD_CONTROL);
    if (GetKeyState(VK_MENU) & BITFLAG(15)) ADD_KEYMOD(KEYMOD_ALT);
    if (GetKeyState(VK_LWIN) & BITFLAG(15)) ADD_KEYMOD(KEYMOD_LEFT_GUI);
    if (GetKeyState(VK_RWIN) & BITFLAG(15)) ADD_KEYMOD(KEYMOD_RIGHT_GUI);
    // ? ALTGR is not there?
    if (GetKeyState(VK_CAPITAL) & BITFLAG(0)) ADD_KEYMOD(KEYMOD_CAPS_LOCK);
    if (GetKeyState(VK_NUMLOCK) & BITFLAG(0)) ADD_KEYMOD(KEYMOD_NUM_LOCK);
    if (GetKeyState(VK_SCROLL) & BITFLAG(0)) ADD_KEYMOD(KEYMOD_SCROLL_LOCK);
}

/**
 * @brief Update the modifiers state.
 *
 * @param virtual_keycode The keycode whose state changed
 * @param pressed Whether it is pressed or not
 */
void updateKeymodsState(u16 virtual_keycode, b8 pressed) {
    Keymod mod = KEYMOD_NONE;
    Keymod lock = KEYMOD_NONE;
    switch (virtual_keycode) {
        case VK_SHIFT:
            mod = KEYMOD_SHIFT;
            break;
        case VK_CONTROL:
            mod = KEYMOD_CONTROL;
            break;
        case VK_MENU:
            mod = KEYMOD_ALT;
            break;
        case VK_LWIN:
            mod = KEYMOD_LEFT_GUI;
            break;
        case VK_RWIN:
            mod = KEYMOD_RIGHT_GUI;
            break;
        case VK_CAPITAL:
            lock = KEYMOD_CAPS_LOCK;
            break;
        case VK_NUMLOCK:
            lock = KEYMOD_NUM_LOCK;
            break;
        case VK_SCROLL:
            lock = KEYMOD_SCROLL_LOCK;
            break;
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
 * @brief Get the current state of the keymods.
 *
 * @return Active Keymods.
 */
u32 getKeymods(void) {
    return keymod_state;
}

#endif
