#include "keyboard.h"

#include "core/logger.h"

static const Keycode scan_to_key[] = {
    [SCANCODE_NONE] = KEYCODE_NONE,

    [SCANCODE_ENTER] = KEYCODE_ENTER,
    [SCANCODE_ESCAPE] = KEYCODE_ESCAPE,
    [SCANCODE_BACKSPACE] = KEYCODE_BACKSPACE,
    [SCANCODE_TAB] = KEYCODE_TAB,
    [SCANCODE_SPACEBAR] = KEYCODE_SPACE,

    [SCANCODE_MINUS] = KEYCODE_MINUS,
    [SCANCODE_EQUALS] = KEYCODE_EQUALS,

    [SCANCODE_LEFT_BRACKET] = KEYCODE_LEFT_BRACKET,
    [SCANCODE_RIGHT_BRACKET] = KEYCODE_RIGHT_BRACKET,

    [SCANCODE_BACKSLASH] = KEYCODE_BACKSLASH,

    [SCANCODE_SEMICOLON] = KEYCODE_SEMICOLON,
    [SCANCODE_APOSTROPHE] = KEYCODE_SINGLE_QUOTE,

    [SCANCODE_GRAVE] = KEYCODE_GRAVE,

    [SCANCODE_COMMA] = KEYCODE_COMMA,
    [SCANCODE_PERIOD] = KEYCODE_PERIOD,
    [SCANCODE_SLASH] = KEYCODE_SLASH,

    [SCANCODE_1] = KEYCODE_EXCLAM,
    [SCANCODE_2] = KEYCODE_AT,
    [SCANCODE_3] = KEYCODE_HASH,
    [SCANCODE_4] = KEYCODE_DOLLER,
    [SCANCODE_5] = KEYCODE_PERCENT,
    [SCANCODE_6] = KEYCODE_CARET,
    [SCANCODE_7] = KEYCODE_AMPERSAND,
    [SCANCODE_8] = KEYCODE_ASTERISK,
    [SCANCODE_9] = KEYCODE_LEFT_PAREN,
    [SCANCODE_0] = KEYCODE_RIGHT_PAREN,

    [SCANCODE_DELETE] = KEYCODE_DELETE,

    // Should this be something else?
    [SCANCODE_NON_US_HASH] = KEYCODE_NONE,
    [SCANCODE_NON_US_BACKSLASH] = KEYCODE_NONE,
    [SCANCODE_LOCKING_CAPS_LOCK] = KEYCODE_NONE,
    [SCANCODE_LOCKING_NUM_LOCK] = KEYCODE_NONE,
    [SCANCODE_LOCKING_SCROLL_LOCK] = KEYCODE_NONE,

    [SCANCODE_INTERNATIONAL_1] = KEYCODE_NONE,
    [SCANCODE_INTERNATIONAL_2] = KEYCODE_NONE,
    [SCANCODE_INTERNATIONAL_3] = KEYCODE_NONE,
    [SCANCODE_INTERNATIONAL_4] = KEYCODE_NONE,
    [SCANCODE_INTERNATIONAL_15] = KEYCODE_NONE,
    [SCANCODE_INTERNATIONAL_16] = KEYCODE_NONE,
    [SCANCODE_INTERNATIONAL_17] = KEYCODE_NONE,
    [SCANCODE_INTERNATIONAL_18] = KEYCODE_NONE,
    [SCANCODE_INTERNATIONAL_19] = KEYCODE_NONE,

    [SCANCODE_LANG_1] = KEYCODE_NONE,
    [SCANCODE_LANG_2] = KEYCODE_NONE,
    [SCANCODE_LANG_3] = KEYCODE_NONE,
    [SCANCODE_LANG_4] = KEYCODE_NONE,
    [SCANCODE_LANG_5] = KEYCODE_NONE,
    [SCANCODE_LANG_6] = KEYCODE_NONE,
    [SCANCODE_LANG_7] = KEYCODE_NONE,
    [SCANCODE_LANG_8] = KEYCODE_NONE,
    [SCANCODE_LANG_9] = KEYCODE_NONE,
};

static const Scancode key_to_scan[] = {
    [KEYCODE_NONE] = SCANCODE_NONE,

    [KEYCODE_BACKSPACE] = SCANCODE_BACKSPACE,
    [KEYCODE_TAB] = SCANCODE_TAB,
    [KEYCODE_ENTER] = SCANCODE_ENTER,
    [KEYCODE_ESCAPE] = SCANCODE_ESCAPE,
    [KEYCODE_SPACE] = SCANCODE_SPACEBAR,

    [KEYCODE_DOUBLE_QUOTE] = SCANCODE_APOSTROPHE,
    [KEYCODE_SINGLE_QUOTE] = SCANCODE_APOSTROPHE,

    [KEYCODE_EXCLAM] = SCANCODE_1,
    [KEYCODE_AT] = SCANCODE_2,
    [KEYCODE_HASH] = SCANCODE_3,
    [KEYCODE_DOLLER] = SCANCODE_4,
    [KEYCODE_PERCENT] = SCANCODE_5,
    [KEYCODE_CARET] = SCANCODE_6,
    [KEYCODE_AMPERSAND] = SCANCODE_7,
    [KEYCODE_ASTERISK] = SCANCODE_8,
    [KEYCODE_LEFT_PAREN] = SCANCODE_9,
    [KEYCODE_RIGHT_PAREN] = SCANCODE_0,

    [KEYCODE_PLUS] = SCANCODE_EQUALS,
    [KEYCODE_EQUALS] = SCANCODE_EQUALS,

    [KEYCODE_MINUS] = SCANCODE_MINUS,
    [KEYCODE_UNDERSCORE] = SCANCODE_MINUS,

    [KEYCODE_COMMA] = SCANCODE_COMMA,
    [KEYCODE_LESS_THAN] = SCANCODE_COMMA,

    [KEYCODE_PERIOD] = SCANCODE_PERIOD,
    [KEYCODE_GREATER_THAN] = SCANCODE_PERIOD,

    [KEYCODE_SLASH] = SCANCODE_SLASH,
    [KEYCODE_QUESTION_MARK] = SCANCODE_SLASH,

    [KEYCODE_0] = SCANCODE_0,

    [KEYCODE_COLON] = SCANCODE_SEMICOLON,
    [KEYCODE_SEMICOLON] = SCANCODE_SEMICOLON,

    [KEYCODE_LEFT_BRACKET] = SCANCODE_LEFT_BRACKET,
    [KEYCODE_LEFT_BRACE] = SCANCODE_LEFT_BRACKET,

    [KEYCODE_RIGHT_BRACKET] = SCANCODE_RIGHT_BRACKET,
    [KEYCODE_RIGHT_BRACE] = SCANCODE_RIGHT_BRACKET,

    [KEYCODE_BACKSLASH] = SCANCODE_BACKSLASH,
    [KEYCODE_VERTICAL_BAR] = SCANCODE_BACKSLASH,

    [KEYCODE_GRAVE] = SCANCODE_GRAVE,
    [KEYCODE_TILDE] = SCANCODE_GRAVE,

    [KEYCODE_DELETE] = SCANCODE_DELETE,
};

/**
 * @brief Get the keycode from the Scancode and the modifier state.
 *
 * @param sc Scancode
 * @param mod Keymod
 *
 * @return Returns the respective Keycode for given Scancode and modifiers.
 */
Keycode scancodeToKeycode(Scancode sc, Keymod mod) {
    if (sc == SCANCODE_MAX_SCANCODE) return KEYCODE_NONE;

    if ((sc >= SCANCODE_CAPS_LOCK && sc < SCANCODE_DELETE)
        || (sc > SCANCODE_DELETE && sc < SCANCODE_NON_US_BACKSLASH)
        || (sc >= SCANCODE_APPLICATION && sc <= SCANCODE_VOLUME_DOWN)
        || (sc >= SCANCODE_KEYPAD_COMMA && sc <= SCANCODE_KEYPAD_EQUALS_AS400)
        || (sc >= SCANCODE_ALTERNATE_ERASE && sc <= SCANCODE_RIGHT_GUI))
        // KeyCodes are just OR'd with SCANCODE_MASK
        return (sc | SCANCODE_MASK);

    if (sc >= SCANCODE_A && sc <= SCANCODE_Z) {  // Letters
        switch (mod & (KEYMOD_SHIFT | KEYMOD_CAPS_LOCK)) {
            case KEYMOD_SHIFT:  // Only shift
            case KEYMOD_CAPS_LOCK:  // Only caps
                // Capital letters
                return KEYCODE_A + (sc - SCANCODE_A);
            case KEYMOD_CAPS_LOCK | KEYMOD_SHIFT:  // caps on and shift
            case KEYMOD_NONE:  // neither caps nor shift
                // Small letters
                return KEYCODE_a + (sc - SCANCODE_A);
            default:
                sError("Error in the logic of scancodeToKeycode function "
                       "while "
                       "checking whether to return Capital or Small letter.");
                return KEYCODE_NONE;
        }
    }

    if (mod & KEYMOD_SHIFT) {
        switch (sc) {
            case SCANCODE_MINUS:
                return KEYCODE_UNDERSCORE;
            case SCANCODE_EQUALS:
                return KEYCODE_PLUS;
            case SCANCODE_LEFT_BRACKET:
                return KEYCODE_LEFT_BRACE;
            case SCANCODE_RIGHT_BRACKET:
                return KEYCODE_RIGHT_BRACE;
            case SCANCODE_BACKSLASH:
                return KEYCODE_VERTICAL_BAR;
            case SCANCODE_SEMICOLON:
                return KEYCODE_COLON;
            case SCANCODE_APOSTROPHE:
                return KEYCODE_DOUBLE_QUOTE;
            case SCANCODE_GRAVE:
                return KEYCODE_TILDE;
            case SCANCODE_COMMA:
                return KEYCODE_LESS_THAN;
            case SCANCODE_PERIOD:
                return KEYCODE_GREATER_THAN;
            case SCANCODE_SLASH:
                return KEYCODE_QUESTION_MARK;
            default:
                // Might be single state keys or symbols in the number keys
                return scan_to_key[sc];
        }
    } else {
        if (sc >= SCANCODE_1 && sc <= SCANCODE_9)
            return KEYCODE_1 + (sc - SCANCODE_1);
        else if (sc == SCANCODE_0) return KEYCODE_0;
    }

    return scan_to_key[sc];
}

/**
 * @brief Get the Scancode from the Keycode.
 *
 * @param kc Keycode
 *
 * @return Returns the respective Scancode for given Keycode.
 */
Scancode keycodeToScancode(Keycode kc) {
    if (kc == KEYCODE_MAX_KEYCODE) return SCANCODE_NONE;

    if (kc >= KEYCODE_A && kc <= KEYCODE_Z)
        return SCANCODE_A + (kc - KEYCODE_A);

    if (kc >= KEYCODE_a && kc <= KEYCODE_z)
        return SCANCODE_A + (kc - KEYCODE_a);

    if (kc >= KEYCODE_1 && kc <= KEYCODE_9)
        return SCANCODE_1 + (kc - KEYCODE_1);

    if (kc >= KEYCODE_CAPS_LOCK) return (~SCANCODE_MASK) & kc;

    return key_to_scan[kc];
}
