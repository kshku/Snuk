#pragma once

#include "../../window.h"

#if defined(SPLATFORM_WINDOWING_X11_XCB) \
    || defined(SPLATFORM_WINDOWING_X11_XLIB)
    #include "defines.h"
    #include "input/keyboard/keycode.h"
    #include "input/keyboard/scancode.h"

typedef struct XKeyNameType {
        c8 name[4];
} XKeyNameType;

typedef struct XKeyAliasNameType {
        c8 real[4];
        c8 alias[4];
} XKeyAliasNameType;

typedef struct mapFunctionParams {
        XKeyNameType *key_names;
        XKeyAliasNameType *key_aliases;
        u32 num_key_aliases;
        u8 min_key_code;
        u8 max_key_code;
        b8 key_names_start_from_min_key_code;
} mapFunctionParams;

Keycode XKeySymToKeycode(u64 sym);

void mapXKeyCodesToScancodes(mapFunctionParams params, Scancode *map);

#endif
