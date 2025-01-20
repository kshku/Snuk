#pragma once

#include "defines.h"
#include "keyboard/keyboard.h"
#include "mouse/mouse.h"

b8 initializeInput(u64 *size, void *state);

void shutdownInput(void);

void inputUpdate(void);

// SAPI b8 inputIsKeyDown(Scancode sc);
// SAPI b8 inputIsKeyUp(Scancode sc);
// SAPI b8 inputWasKeyDown(Scancode sc);
// SAPI b8 inputWasKeyUp(Scancode sc);

void inputProcessKey(Scancode sc, Keycode kc, u32 mod, b8 pressed, b8 repeat);

// SAPI b8 inputIsButtonDown(Button b);
// SAPI b8 inputIsButtonUp(Button b);
// SAPI b8 inputWasButtonDown(Button b);
// SAPI b8 inputWasButtonUp(Button b);
// SAPI b8 inputIsScrolling(Scroll direction);
// SAPI b8 inputWasScrolling(Scroll direction);

void inputProcessButton(Button b, f64 x, f64 y, u32 mod, b8 pressed);
void inputProcessScroll(Scroll direction, u32 mod, u32 delta);

void inputProcessPointerMotion(f64 x, f64 y);
