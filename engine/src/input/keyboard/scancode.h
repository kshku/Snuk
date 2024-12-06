#pragma once

#include "defines.h"

#define SCANCODE_PREFIX SCANCODE_
#define DEFINE_SCANCODE(key, value) CONCAT(EXPAND(SCANCODE_PREFIX), key) = value

// NOTE: Following SDL's method. One is physical key code and another one is
// NOTE: virtual key code.

// Similar to SDL_Scancode, taken from USB HID Usage tables (from 0x07)
// https://github.com/libsdl-org/SDL/blob/main/include/SDL3/SDL_scancode.h
// https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf or
// https://usb.org/sites/default/files/hut1_5.pdf
// Physical keycodes Independent of the language.
typedef enum ScanCode {
    DEFINE_SCANCODE(NONE, 0x00),

    DEFINE_SCANCODE(A, 0x04),
    DEFINE_SCANCODE(B, 0x05),
    DEFINE_SCANCODE(C, 0x06),
    DEFINE_SCANCODE(D, 0x07),
    DEFINE_SCANCODE(E, 0x08),
    DEFINE_SCANCODE(F, 0x09),
    DEFINE_SCANCODE(G, 0x0A),
    DEFINE_SCANCODE(H, 0x0B),
    DEFINE_SCANCODE(I, 0x0C),
    DEFINE_SCANCODE(J, 0x0D),
    DEFINE_SCANCODE(K, 0x0E),
    DEFINE_SCANCODE(L, 0x0F),
    DEFINE_SCANCODE(M, 0x10),
    DEFINE_SCANCODE(N, 0x11),
    DEFINE_SCANCODE(O, 0x12),
    DEFINE_SCANCODE(P, 0x13),
    DEFINE_SCANCODE(Q, 0x14),
    DEFINE_SCANCODE(R, 0x15),
    DEFINE_SCANCODE(S, 0x16),
    DEFINE_SCANCODE(T, 0x17),
    DEFINE_SCANCODE(U, 0x18),
    DEFINE_SCANCODE(V, 0x19),
    DEFINE_SCANCODE(W, 0x1A),
    DEFINE_SCANCODE(X, 0x1B),
    DEFINE_SCANCODE(Y, 0x1C),
    DEFINE_SCANCODE(Z, 0x1D),

    DEFINE_SCANCODE(1, 0x1E),
    DEFINE_SCANCODE(2, 0x1F),
    DEFINE_SCANCODE(3, 0x20),
    DEFINE_SCANCODE(4, 0x21),
    DEFINE_SCANCODE(5, 0x22),
    DEFINE_SCANCODE(6, 0x23),
    DEFINE_SCANCODE(7, 0x24),
    DEFINE_SCANCODE(8, 0x25),
    DEFINE_SCANCODE(9, 0x26),
    DEFINE_SCANCODE(0, 0x27),

    DEFINE_SCANCODE(ENTER, 0x28),
    DEFINE_SCANCODE(ESCAPE, 0x29),
    DEFINE_SCANCODE(BACKSPACE, 0x2A),
    DEFINE_SCANCODE(TAB, 0x2B),
    DEFINE_SCANCODE(SPACEBAR, 0x2C),

    DEFINE_SCANCODE(MINUS, 0x2D),
    DEFINE_SCANCODE(EQUALS, 0x2E),

    DEFINE_SCANCODE(LEFT_BRACKET, 0x2F),
    DEFINE_SCANCODE(RIGHT_BRACKET, 0x30),

    DEFINE_SCANCODE(BACKSLASH, 0x31),
    DEFINE_SCANCODE(NON_US_HASH, 0x32),

    DEFINE_SCANCODE(SEMICOLON, 0x33),
    DEFINE_SCANCODE(APOSTROPHE, 0x34),

    DEFINE_SCANCODE(GRAVE, 0x35),

    DEFINE_SCANCODE(COMMA, 0x36),
    DEFINE_SCANCODE(PERIOD, 0x37),
    DEFINE_SCANCODE(SLASH, 0x38),

    DEFINE_SCANCODE(CAPS_LOCK, 0x39),

    DEFINE_SCANCODE(F1, 0x3A),
    DEFINE_SCANCODE(F2, 0x3B),
    DEFINE_SCANCODE(F3, 0x3C),
    DEFINE_SCANCODE(F4, 0x3D),
    DEFINE_SCANCODE(F5, 0x3E),
    DEFINE_SCANCODE(F6, 0x3F),
    DEFINE_SCANCODE(F7, 0x40),
    DEFINE_SCANCODE(F8, 0x41),
    DEFINE_SCANCODE(F9, 0x42),
    DEFINE_SCANCODE(F10, 0x43),
    DEFINE_SCANCODE(F11, 0x44),
    DEFINE_SCANCODE(F12, 0x45),

    DEFINE_SCANCODE(PRINT_SCREEN, 0x46),
    DEFINE_SCANCODE(SCROLL_LOCK, 0x47),

    DEFINE_SCANCODE(PAUSE, 0x48),
    DEFINE_SCANCODE(INSERT, 0x49),
    DEFINE_SCANCODE(HOME, 0x4A),
    DEFINE_SCANCODE(PAGE_UP, 0x4B),
    DEFINE_SCANCODE(DELETE, 0x4C),
    DEFINE_SCANCODE(END, 0x4D),
    DEFINE_SCANCODE(PAGE_DOWN, 0x4E),

    DEFINE_SCANCODE(RIGHT_ARROW, 0x4F),
    DEFINE_SCANCODE(LEFT_ARROW, 0x50),
    DEFINE_SCANCODE(DOWN_ARROW, 0x51),
    DEFINE_SCANCODE(UP_ARROW, 0x52),

    DEFINE_SCANCODE(NUM_LOCK, 0x53),

    DEFINE_SCANCODE(KEYPAD_DIVIDE, 0x54),
    DEFINE_SCANCODE(KEYPAD_MULTIPLY, 0x55),
    DEFINE_SCANCODE(KEYPAD_MINUS, 0x56),
    DEFINE_SCANCODE(KEYPAD_PLUS, 0x57),
    DEFINE_SCANCODE(KEYPAD_ENTER, 0x58),

    DEFINE_SCANCODE(KEYPAD_1, 0x59),
    DEFINE_SCANCODE(KEYPAD_2, 0x5A),
    DEFINE_SCANCODE(KEYPAD_3, 0x5B),
    DEFINE_SCANCODE(KEYPAD_4, 0x5C),
    DEFINE_SCANCODE(KEYPAD_5, 0x5D),
    DEFINE_SCANCODE(KEYPAD_6, 0x5E),
    DEFINE_SCANCODE(KEYPAD_7, 0x5F),
    DEFINE_SCANCODE(KEYPAD_8, 0x60),
    DEFINE_SCANCODE(KEYPAD_9, 0x61),
    DEFINE_SCANCODE(KEYPAD_0, 0x62),

    DEFINE_SCANCODE(KEYPAD_PERIOD, 0x63),
    DEFINE_SCANCODE(NON_US_BACKSLASH, 0x64),

    DEFINE_SCANCODE(APPLICATION, 0x65),
    DEFINE_SCANCODE(POWER,
                    0x66), /* The USB document says this is not a physical key,
                              but from SDL's implementation, got to know that
                              some Mac keyboards have power key*/

    DEFINE_SCANCODE(KEYPAD_EQUALS, 0x67),

    DEFINE_SCANCODE(F13, 0x68),
    DEFINE_SCANCODE(F14, 0x69),
    DEFINE_SCANCODE(F15, 0x6A),
    DEFINE_SCANCODE(F16, 0x6B),
    DEFINE_SCANCODE(F17, 0x6C),
    DEFINE_SCANCODE(F18, 0x6D),
    DEFINE_SCANCODE(F19, 0x6E),
    DEFINE_SCANCODE(F20, 0x6F),
    DEFINE_SCANCODE(F21, 0x70),
    DEFINE_SCANCODE(F22, 0x71),
    DEFINE_SCANCODE(F23, 0x72),
    DEFINE_SCANCODE(F24, 0x73),

    DEFINE_SCANCODE(EXECUTE, 0x74),
    DEFINE_SCANCODE(HELP, 0x75),
    DEFINE_SCANCODE(MENU, 0x76),
    DEFINE_SCANCODE(SELECT, 0x77),
    DEFINE_SCANCODE(STOP, 0x78),
    DEFINE_SCANCODE(AGAIN, 0x79),
    DEFINE_SCANCODE(UNDO, 0x7A),
    DEFINE_SCANCODE(CUT, 0x7B),
    DEFINE_SCANCODE(COPY, 0x7C),
    DEFINE_SCANCODE(PASTE, 0x7D),
    DEFINE_SCANCODE(FIND, 0x7E),
    DEFINE_SCANCODE(MUTE, 0x7F),
    DEFINE_SCANCODE(VOLUME_UP, 0x80),
    DEFINE_SCANCODE(VOLUME_DOWN, 0x81),

    /**
     * According to USB document locking keys are available for legacy support;
     * however, most systems should use the non-locking version of this key
     * (CAPS_LOCK, NUM_LOCK, SCROLL_LOCK). Even in the SDL_scancode.h they have
     * disabled these.
     */
    DEFINE_SCANCODE(LOCKING_CAPS_LOCK, 0x82),
    DEFINE_SCANCODE(LOCKING_NUM_LOCK, 0x83),
    DEFINE_SCANCODE(LOCKING_SCROLL_LOCK, 0x84),

    DEFINE_SCANCODE(KEYPAD_COMMA, 0x85),
    DEFINE_SCANCODE(KEYPAD_EQUALS_AS400, 0x86),

    DEFINE_SCANCODE(INTERNATIONAL_1, 0x87),
    DEFINE_SCANCODE(INTERNATIONAL_2, 0x88),
    DEFINE_SCANCODE(INTERNATIONAL_3, 0x89),
    DEFINE_SCANCODE(INTERNATIONAL_4, 0x8A),
    DEFINE_SCANCODE(INTERNATIONAL_15, 0x8B),
    DEFINE_SCANCODE(INTERNATIONAL_16, 0x8C),
    DEFINE_SCANCODE(INTERNATIONAL_17, 0x8D),
    DEFINE_SCANCODE(INTERNATIONAL_18, 0x8E),
    DEFINE_SCANCODE(INTERNATIONAL_19, 0x8F),

    DEFINE_SCANCODE(LANG_1, 0x90),
    DEFINE_SCANCODE(LANG_2, 0x91),
    DEFINE_SCANCODE(LANG_3, 0x92),
    DEFINE_SCANCODE(LANG_4, 0x93),
    DEFINE_SCANCODE(LANG_5, 0x94),
    DEFINE_SCANCODE(LANG_6, 0x95),
    DEFINE_SCANCODE(LANG_7, 0x96),
    DEFINE_SCANCODE(LANG_8, 0x97),
    DEFINE_SCANCODE(LANG_9, 0x98),

    DEFINE_SCANCODE(ALTERNATE_ERASE, 0x99),
    DEFINE_SCANCODE(SYSREQ, 0x9A),
    DEFINE_SCANCODE(CANCEL, 0x9B),
    DEFINE_SCANCODE(CLEAR, 0x9C),
    DEFINE_SCANCODE(PRIOR, 0x9D),
    DEFINE_SCANCODE(
        RETURN,
        0x9E), /* defined as SDL_SCANCODE_RETURN2 in SDL implementation*/
    DEFINE_SCANCODE(SEPARATOR, 0x9F),
    DEFINE_SCANCODE(OUT, 0xA0),
    DEFINE_SCANCODE(OPER, 0xA1),

    DEFINE_SCANCODE(CLEARAGAIN, 0xA2),
    DEFINE_SCANCODE(CRSEL, 0xA3),
    DEFINE_SCANCODE(EXSEL, 0xA4),

    DEFINE_SCANCODE(KEYPAD_00, 0xB0),
    DEFINE_SCANCODE(KEYPAD_000, 0xB1),

    DEFINE_SCANCODE(THOUSANDS_SEPARATOR, 0xB2),
    DEFINE_SCANCODE(DECIMAL_SEPARATOR, 0xB3),
    DEFINE_SCANCODE(CURRENCY_UNIT, 0xB4),
    DEFINE_SCANCODE(CURRENCY_SUBUNIT, 0xB5),

    DEFINE_SCANCODE(KEYPAD_LEFT_PAREN, 0xB6),
    DEFINE_SCANCODE(KEYPAD_RIGHT_PAREN, 0xB7),
    DEFINE_SCANCODE(KEYPAD_LEFT_BRACE, 0xB8),
    DEFINE_SCANCODE(KEYPAD_RIGHT_BRACE, 0xB9),

    DEFINE_SCANCODE(KEYPAD_TAB, 0xBA),
    DEFINE_SCANCODE(KEYPAD_BACKSPACE, 0xBB),

    DEFINE_SCANCODE(KEYPAD_A, 0xBC),
    DEFINE_SCANCODE(KEYPAD_B, 0xBD),
    DEFINE_SCANCODE(KEYPAD_C, 0xBE),
    DEFINE_SCANCODE(KEYPAD_D, 0xBF),
    DEFINE_SCANCODE(KEYPAD_E, 0xC0),
    DEFINE_SCANCODE(KEYPAD_F, 0xC1),

    DEFINE_SCANCODE(KEYPAD_XOR, 0xC2),
    DEFINE_SCANCODE(KEYPAD_CARET, 0xC3),
    DEFINE_SCANCODE(KEYPAD_PERCENT, 0xC4),
    DEFINE_SCANCODE(KEYPAD_LESS_THAN, 0xC5),
    DEFINE_SCANCODE(KEYPAD_GREATER_THAN, 0xC6),
    DEFINE_SCANCODE(KEYPAD_AMPERSAND, 0xC7),
    DEFINE_SCANCODE(KEYPAD_DOUBLE_AMPERSAND, 0xC8),
    DEFINE_SCANCODE(KEYPAD_VERTICAL_BAR, 0xC9),
    DEFINE_SCANCODE(KEYPAD_DOUBLE_VERTICAL_BAR, 0xCA),
    DEFINE_SCANCODE(KEYPAD_COLON, 0xCB),
    DEFINE_SCANCODE(KEYPAD_HASH, 0xCC),
    DEFINE_SCANCODE(KEYPAD_SPACE, 0xCD),
    DEFINE_SCANCODE(KEYPAD_AT, 0xCE),
    DEFINE_SCANCODE(KEYPAD_EXCLAM, 0xCF),

    DEFINE_SCANCODE(KEYPAD_MEMORY_STORE, 0xD0),
    DEFINE_SCANCODE(KEYPAD_MEMORY_RECALL, 0xD1),
    DEFINE_SCANCODE(KEYPAD_MEMORY_CLEAR, 0xD2),
    DEFINE_SCANCODE(KEYPAD_MEMORY_ADD, 0xD3),
    DEFINE_SCANCODE(KEYPAD_MEMORY_SUBTRACT, 0xD4),
    DEFINE_SCANCODE(KEYPAD_MEMORY_MULTIPLY, 0xD5),
    DEFINE_SCANCODE(KEYPAD_MEMORY_DIVIDE, 0xD6),
    DEFINE_SCANCODE(KEYPAD_PLUS_MINUS, 0xD7),

    DEFINE_SCANCODE(KEYPAD_CLEAR, 0xD8),
    DEFINE_SCANCODE(KEYPAD_CLEAR_ENTRY, 0xD9),

    DEFINE_SCANCODE(KEYPAD_BINARY, 0xDA),
    DEFINE_SCANCODE(KEYPAD_OCTAL, 0xDB),
    DEFINE_SCANCODE(KEYPAD_DECIMAL, 0xDC),
    DEFINE_SCANCODE(KEYPAD_HEXADECIMAL, 0xDD),

    DEFINE_SCANCODE(LEFT_CONTROL, 0xE0),
    DEFINE_SCANCODE(LEFT_SHIFT, 0xE1),
    DEFINE_SCANCODE(LEFT_ALT, 0xE2),
    DEFINE_SCANCODE(LEFT_GUI, 0xE3),
    DEFINE_SCANCODE(RIGHT_CONTROL, 0xE4),
    DEFINE_SCANCODE(RIGHT_SHIFT, 0xE5),
    DEFINE_SCANCODE(RIGHT_ALT, 0xE6),
    DEFINE_SCANCODE(RIGHT_GUI, 0xE7),

    SCANCODE_MAX_SCANCODE
} ScanCode;