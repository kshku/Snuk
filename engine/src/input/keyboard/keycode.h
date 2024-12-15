#pragma once

#include "defines.h"
#include "scancode.h"

#define KEYCODE_PREFIX KEYCODE_
#define DEFINE_KEYCODE(key, value) CONCAT_EXPANDED(KEYCODE_PREFIX, key) = value

// NOTE: Expecting the size of enum constant to be 4 bytes or 32 bits
#define SCANCODE_MASK (1 << 30)

// /**
//  * @brief Keycode from the Scancode with given Keycode name.
//  */
// #define DEFINE_FROM_SCANCODE_WITH_NAME(key, scan) \
//     DEFINE_KEYCODE(key, (CONCAT(EXPAND(SCANCODE_PREFIX), scan)) |
//     SCANCODE_MASK)

// /**
//  * @brief Keycode from Scancode, Keycode name same as Scancode name.
//  */
// #define DEFINE_KEYCODE_FROM_SCANCODE(key) \
//     DEFINE_FROM_SCANCODE_WITH_NAME(key, key)

/**
 * @brief Keycode from Scancode, Keycode name same as Scancode name.
 */
#define DEFINE_KEYCODE_FROM_SCANCODE(key) \
    DEFINE_KEYCODE(key, (CONCAT_EXPANDED(SCANCODE_PREFIX, key)) | SCANCODE_MASK)

// NOTE: The virtual key codes.
// Using SDL's approach

typedef enum Keycode {
    DEFINE_KEYCODE(NONE, 0),

    DEFINE_KEYCODE(BACKSPACE, 0x08),
    DEFINE_KEYCODE(TAB, 0x09),
    // DEFINE_KEYCODE(LINE_FEED, 0x0A),  // Don't think this is required.
    DEFINE_KEYCODE(ENTER, 0x0D),
    DEFINE_KEYCODE(ESCAPE, 0x1B),
    DEFINE_KEYCODE(SPACE, 0x20),
    DEFINE_KEYCODE(EXCLAM, 0x21),
    DEFINE_KEYCODE(DOUBLE_QUOTE, 0x22),
    DEFINE_KEYCODE(HASH, 0x23),
    DEFINE_KEYCODE(DOLLER, 0x24),
    DEFINE_KEYCODE(PERCENT, 0x25),
    DEFINE_KEYCODE(AMPERSAND, 0x26),
    DEFINE_KEYCODE(SINGLE_QUOTE, 0x27),
    DEFINE_KEYCODE(LEFT_PAREN, 0x28),
    DEFINE_KEYCODE(RIGHT_PAREN, 0x29),
    DEFINE_KEYCODE(ASTERISK, 0x2A),
    DEFINE_KEYCODE(PLUS, 0x2B),
    DEFINE_KEYCODE(COMMA, 0x2C),
    DEFINE_KEYCODE(MINUS, 0x2D),
    DEFINE_KEYCODE(PERIOD, 0x2E),
    DEFINE_KEYCODE(SLASH, 0x2F),

    DEFINE_KEYCODE(0, 0x30),
    DEFINE_KEYCODE(1, 0x31),
    DEFINE_KEYCODE(2, 0x32),
    DEFINE_KEYCODE(3, 0x33),
    DEFINE_KEYCODE(4, 0x34),
    DEFINE_KEYCODE(5, 0x35),
    DEFINE_KEYCODE(6, 0x36),
    DEFINE_KEYCODE(7, 0x37),
    DEFINE_KEYCODE(8, 0x38),
    DEFINE_KEYCODE(9, 0x39),

    DEFINE_KEYCODE(COLON, 0x3A),
    DEFINE_KEYCODE(SEMICOLON, 0x3B),
    DEFINE_KEYCODE(LESS_THAN, 0x3C),
    DEFINE_KEYCODE(EQUALS, 0x3D),
    DEFINE_KEYCODE(GREATER_THAN, 0x3E),
    DEFINE_KEYCODE(QUESTION_MARK, 0x3F),
    DEFINE_KEYCODE(AT, 0x40),

    DEFINE_KEYCODE(A, 0x41),
    DEFINE_KEYCODE(B, 0x42),
    DEFINE_KEYCODE(C, 0x43),
    DEFINE_KEYCODE(D, 0x44),
    DEFINE_KEYCODE(E, 0x45),
    DEFINE_KEYCODE(F, 0x46),
    DEFINE_KEYCODE(G, 0x47),
    DEFINE_KEYCODE(H, 0x48),
    DEFINE_KEYCODE(I, 0x49),
    DEFINE_KEYCODE(J, 0x4A),
    DEFINE_KEYCODE(K, 0x4B),
    DEFINE_KEYCODE(L, 0x4C),
    DEFINE_KEYCODE(M, 0x4D),
    DEFINE_KEYCODE(N, 0x4E),
    DEFINE_KEYCODE(O, 0x4F),
    DEFINE_KEYCODE(P, 0x50),
    DEFINE_KEYCODE(Q, 0x51),
    DEFINE_KEYCODE(R, 0x52),
    DEFINE_KEYCODE(S, 0x53),
    DEFINE_KEYCODE(T, 0x54),
    DEFINE_KEYCODE(U, 0x55),
    DEFINE_KEYCODE(V, 0x56),
    DEFINE_KEYCODE(W, 0x57),
    DEFINE_KEYCODE(X, 0x58),
    DEFINE_KEYCODE(Y, 0x59),
    DEFINE_KEYCODE(Z, 0x5A),

    DEFINE_KEYCODE(LEFT_BRACKET, 0x5B),
    DEFINE_KEYCODE(BACKSLASH, 0x5C),
    DEFINE_KEYCODE(RIGHT_BRACKET, 0x5D),
    DEFINE_KEYCODE(CARET, 0x5E),
    DEFINE_KEYCODE(UNDERSCORE, 0x5F),
    DEFINE_KEYCODE(GRAVE, 0x60),

    DEFINE_KEYCODE(a, 0x61),
    DEFINE_KEYCODE(b, 0x62),
    DEFINE_KEYCODE(c, 0x63),
    DEFINE_KEYCODE(d, 0x64),
    DEFINE_KEYCODE(e, 0x65),
    DEFINE_KEYCODE(f, 0x66),
    DEFINE_KEYCODE(g, 0x67),
    DEFINE_KEYCODE(h, 0x68),
    DEFINE_KEYCODE(i, 0x69),
    DEFINE_KEYCODE(j, 0x6A),
    DEFINE_KEYCODE(k, 0x6B),
    DEFINE_KEYCODE(l, 0x6C),
    DEFINE_KEYCODE(m, 0x6D),
    DEFINE_KEYCODE(n, 0x6E),
    DEFINE_KEYCODE(o, 0x6F),
    DEFINE_KEYCODE(p, 0x70),
    DEFINE_KEYCODE(q, 0x71),
    DEFINE_KEYCODE(r, 0x72),
    DEFINE_KEYCODE(s, 0x73),
    DEFINE_KEYCODE(t, 0x74),
    DEFINE_KEYCODE(u, 0x75),
    DEFINE_KEYCODE(v, 0x76),
    DEFINE_KEYCODE(w, 0x77),
    DEFINE_KEYCODE(x, 0x78),
    DEFINE_KEYCODE(y, 0x79),
    DEFINE_KEYCODE(z, 0x7A),

    DEFINE_KEYCODE(LEFT_BRACE, 0x7B),
    DEFINE_KEYCODE(VERTICAL_BAR, 0x7C),
    DEFINE_KEYCODE(RIGHT_BRACE, 0x7D),
    DEFINE_KEYCODE(TILDE, 0x7E),
    DEFINE_KEYCODE(DELETE, 0x7F),

    DEFINE_KEYCODE_FROM_SCANCODE(CAPS_LOCK),

    DEFINE_KEYCODE_FROM_SCANCODE(F1),
    DEFINE_KEYCODE_FROM_SCANCODE(F2),
    DEFINE_KEYCODE_FROM_SCANCODE(F3),
    DEFINE_KEYCODE_FROM_SCANCODE(F4),
    DEFINE_KEYCODE_FROM_SCANCODE(F5),
    DEFINE_KEYCODE_FROM_SCANCODE(F6),
    DEFINE_KEYCODE_FROM_SCANCODE(F7),
    DEFINE_KEYCODE_FROM_SCANCODE(F8),
    DEFINE_KEYCODE_FROM_SCANCODE(F9),
    DEFINE_KEYCODE_FROM_SCANCODE(F10),
    DEFINE_KEYCODE_FROM_SCANCODE(F11),
    DEFINE_KEYCODE_FROM_SCANCODE(F12),

    DEFINE_KEYCODE_FROM_SCANCODE(PRINT_SCREEN),
    DEFINE_KEYCODE_FROM_SCANCODE(SCROLL_LOCK),

    DEFINE_KEYCODE_FROM_SCANCODE(PAUSE),
    DEFINE_KEYCODE_FROM_SCANCODE(INSERT),
    DEFINE_KEYCODE_FROM_SCANCODE(HOME),
    DEFINE_KEYCODE_FROM_SCANCODE(PAGE_UP),
    DEFINE_KEYCODE_FROM_SCANCODE(END),
    DEFINE_KEYCODE_FROM_SCANCODE(PAGE_DOWN),

    DEFINE_KEYCODE_FROM_SCANCODE(RIGHT_ARROW),
    DEFINE_KEYCODE_FROM_SCANCODE(LEFT_ARROW),
    DEFINE_KEYCODE_FROM_SCANCODE(DOWN_ARROW),
    DEFINE_KEYCODE_FROM_SCANCODE(UP_ARROW),

    DEFINE_KEYCODE_FROM_SCANCODE(NUM_LOCK),

    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_DIVIDE),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_MULTIPLY),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_MINUS),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_PLUS),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_ENTER),

    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_0),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_1),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_2),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_3),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_4),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_5),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_6),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_7),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_8),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_9),

    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_PERIOD),

    // Add all the keycodes defined in Scancode which are not already defined.

    DEFINE_KEYCODE_FROM_SCANCODE(APPLICATION),
    DEFINE_KEYCODE_FROM_SCANCODE(POWER),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_EQUALS),

    DEFINE_KEYCODE_FROM_SCANCODE(F13),
    DEFINE_KEYCODE_FROM_SCANCODE(F14),
    DEFINE_KEYCODE_FROM_SCANCODE(F15),
    DEFINE_KEYCODE_FROM_SCANCODE(F16),
    DEFINE_KEYCODE_FROM_SCANCODE(F17),
    DEFINE_KEYCODE_FROM_SCANCODE(F18),
    DEFINE_KEYCODE_FROM_SCANCODE(F19),
    DEFINE_KEYCODE_FROM_SCANCODE(F20),
    DEFINE_KEYCODE_FROM_SCANCODE(F21),
    DEFINE_KEYCODE_FROM_SCANCODE(F22),
    DEFINE_KEYCODE_FROM_SCANCODE(F23),
    DEFINE_KEYCODE_FROM_SCANCODE(F24),

    DEFINE_KEYCODE_FROM_SCANCODE(EXECUTE),
    DEFINE_KEYCODE_FROM_SCANCODE(HELP),
    DEFINE_KEYCODE_FROM_SCANCODE(MENU),
    DEFINE_KEYCODE_FROM_SCANCODE(SELECT),
    DEFINE_KEYCODE_FROM_SCANCODE(STOP),
    DEFINE_KEYCODE_FROM_SCANCODE(AGAIN),
    DEFINE_KEYCODE_FROM_SCANCODE(UNDO),
    DEFINE_KEYCODE_FROM_SCANCODE(CUT),
    DEFINE_KEYCODE_FROM_SCANCODE(COPY),
    DEFINE_KEYCODE_FROM_SCANCODE(PASTE),
    DEFINE_KEYCODE_FROM_SCANCODE(FIND),

    DEFINE_KEYCODE_FROM_SCANCODE(MUTE),
    DEFINE_KEYCODE_FROM_SCANCODE(VOLUME_UP),
    DEFINE_KEYCODE_FROM_SCANCODE(VOLUME_DOWN),

    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_COMMA),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_EQUALS_AS400),
    DEFINE_KEYCODE_FROM_SCANCODE(ALTERNATE_ERASE),
    DEFINE_KEYCODE_FROM_SCANCODE(SYSREQ),
    DEFINE_KEYCODE_FROM_SCANCODE(CANCEL),
    DEFINE_KEYCODE_FROM_SCANCODE(CLEAR),

    DEFINE_KEYCODE_FROM_SCANCODE(PRIOR),
    DEFINE_KEYCODE_FROM_SCANCODE(RETURN),
    DEFINE_KEYCODE_FROM_SCANCODE(SEPARATOR),
    DEFINE_KEYCODE_FROM_SCANCODE(OUT),
    DEFINE_KEYCODE_FROM_SCANCODE(OPER),
    DEFINE_KEYCODE_FROM_SCANCODE(CLEARAGAIN),
    DEFINE_KEYCODE_FROM_SCANCODE(CRSEL),
    DEFINE_KEYCODE_FROM_SCANCODE(EXSEL),

    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_00),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_000),

    DEFINE_KEYCODE_FROM_SCANCODE(THOUSANDS_SEPARATOR),
    DEFINE_KEYCODE_FROM_SCANCODE(DECIMAL_SEPARATOR),
    DEFINE_KEYCODE_FROM_SCANCODE(CURRENCY_UNIT),
    DEFINE_KEYCODE_FROM_SCANCODE(CURRENCY_SUBUNIT),

    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_LEFT_PAREN),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_RIGHT_PAREN),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_LEFT_BRACE),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_RIGHT_BRACE),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_TAB),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_BACKSPACE),

    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_A),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_B),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_C),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_D),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_E),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_F),

    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_XOR),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_CARET),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_PERCENT),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_LESS_THAN),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_GREATER_THAN),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_AMPERSAND),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_DOUBLE_AMPERSAND),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_VERTICAL_BAR),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_DOUBLE_VERTICAL_BAR),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_COLON),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_HASH),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_SPACE),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_AT),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_EXCLAM),

    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_MEMORY_STORE),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_MEMORY_RECALL),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_MEMORY_CLEAR),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_MEMORY_ADD),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_MEMORY_SUBTRACT),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_MEMORY_MULTIPLY),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_MEMORY_DIVIDE),

    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_PLUS_MINUS),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_CLEAR),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_CLEAR_ENTRY),

    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_BINARY),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_OCTAL),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_DECIMAL),
    DEFINE_KEYCODE_FROM_SCANCODE(KEYPAD_HEXADECIMAL),

    DEFINE_KEYCODE_FROM_SCANCODE(LEFT_CONTROL),
    DEFINE_KEYCODE_FROM_SCANCODE(LEFT_SHIFT),
    DEFINE_KEYCODE_FROM_SCANCODE(LEFT_ALT),
    DEFINE_KEYCODE_FROM_SCANCODE(LEFT_GUI),

    DEFINE_KEYCODE_FROM_SCANCODE(RIGHT_CONTROL),
    DEFINE_KEYCODE_FROM_SCANCODE(RIGHT_SHIFT),
    DEFINE_KEYCODE_FROM_SCANCODE(RIGHT_ALT),
    DEFINE_KEYCODE_FROM_SCANCODE(RIGHT_GUI),

    KEYCODE_MAX_KEYCODE
} Keycode;

#define KEYMOD_PREFIX KEYMOD_

#define DEFINE_KEYMOD(mod, value) CONCAT_EXPANDED(KEYMOD_PREFIX, mod) = value

typedef enum Keymod {
    DEFINE_KEYMOD(NONE, 0),

    DEFINE_KEYMOD(LEFT_SHIFT, BITFLAG(0)),
    DEFINE_KEYMOD(RIGHT_SHIFT, BITFLAG(1)),

    DEFINE_KEYMOD(LEFT_CONTROL, BITFLAG(2)),
    DEFINE_KEYMOD(RIGHT_CONTROL, BITFLAG(3)),

    DEFINE_KEYMOD(LEFT_ALT, BITFLAG(4)),
    DEFINE_KEYMOD(RIGHT_ALT, BITFLAG(5)),

    DEFINE_KEYMOD(LEFT_GUI, BITFLAG(6)),
    DEFINE_KEYMOD(RIGHT_GUI, BITFLAG(7)),

#define Keymod(key) CONCAT_EXPANDED(KEYMOD_PREFIX, key)
    DEFINE_KEYMOD(SHIFT, (Keymod(LEFT_SHIFT) | Keymod(RIGHT_SHIFT))),
    DEFINE_KEYMOD(CONTROL, (Keymod(LEFT_CONTROL) | Keymod(RIGHT_CONTROL))),
    DEFINE_KEYMOD(ALT, (Keymod(LEFT_ALT) | Keymod(RIGHT_ALT))),
    DEFINE_KEYMOD(GUI, (Keymod(LEFT_GUI) | Keymod(RIGHT_GUI))),
#undef Keymod

    DEFINE_KEYMOD(NUM_LOCK, BITFLAG(8)),  // num lock is on
    DEFINE_KEYMOD(CAPS_LOCK, BITFLAG(9)),  // caps lock is on
    DEFINE_KEYMOD(SCROLL_LOCK, BITFLAG(10)),  // scroll lock is on

    // #define SDL_KMOD_MODE 0x4000u /**< the !AltGr key is down. */

} Keymod;

STATIC_ASSERT(sizeof(KEYCODE_MAX_KEYCODE) == 4,
              "Expected size of the enum constant is 4 bytes.");
