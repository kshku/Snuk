#pragma once

#include "defines.h"
#include "keyboard/keyboard.h"

b8 initializeInput(u64 *size, void *state);

void shutdownInput(void *state);

void inputUpdate(void);

SAPI b8 inputIsKeyDown(ScanCode sc);
SAPI b8 inputIsKeyUp(ScanCode sc);
SAPI b8 inputWasKeyDown(ScanCode sc);
SAPI b8 inputWasKeyUp(ScanCode sc);

void inputProcessKey(ScanCode sc, b8 pressed);
