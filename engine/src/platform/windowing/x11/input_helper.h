#pragma once

#include "defines.h"
#include "input/keyboard/keycode.h"
#include "input/keyboard/scancode.h"

typedef struct XKeyNameType {
        char name[4];
} XKeyNameType;

typedef struct XKeyAliasNameType {
        char real[4];
        char alias[4];
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
