#include "input.h"

#include "core/assertions.h"
#include "core/event.h"
#include "core/memory/memory.h"

// TODO: How to maintain the states
// TODO: Move the functions to the keyboard.h and mouse.h files

typedef struct InputState {
        KeyboardState keyboard_current;
        KeyboardState keyboard_previous;
        MouseState mouse_current;
        MouseState mouse_previous;
} InputState;

static InputState *input_state;

/**
 * @brief Initialize Input system.
 *
 * Call with state NULL to get the size to be allocated and call once again with
 * pointer to the allocated memory to actually initialize.
 *
 * @param size The size of allocation required by input system
 * @param state Pointer to the allocated memory for input system
 *
 * @return Returns true if initialized successfully, else false.
 */
b8 initializeInput(u64 *size, void *state) {
    sassert_msg(!input_state, "Initializing Input system twice?");

    // I don't think heap allocation is requird
    *size = sizeof(InputState);

    if (!state) return false;

    input_state = (InputState *)state;

    return true;
}

/**
 * @brief Shut down the input system.
 */
void shutdownInput(void) {
    sassert_msg(
        input_state,
        "Input system is not initialized or calling shutdownInput twice?");
}

/**
 * @brief Update the input state.
 */
void inputUpdate(void) {
    sassert_msg(input_state, "Input system is not initialized.");

    sMemCopy(&input_state->keyboard_previous, &input_state->keyboard_current,
             sizeof(KeyboardState));
    sMemCopy(&input_state->mouse_previous, &input_state->mouse_current,
             sizeof(MouseState));
}

// /**
//  * @brief Check whether the given key is down now.
//  *
//  * @param sc Scancode of the key to be checked.
//  *
//  * @return true if is down, else false.
//  */
// b8 inputIsKeyDown(Scancode sc) {
//     sassert_msg(input_state, "Input system is not initialized.");
//     return input_state->keyboard_current.scancodes[sc];
// }

// /**
//  * @brief Check whether the given key is up now.
//  *
//  * @param sc Scancode of the key to be checked.
//  *
//  * @return true if is up, else false.
//  */
// b8 inputIsKeyUp(Scancode sc) {
//     sassert_msg(input_state, "Input system is not initialized.");
//     return !input_state->keyboard_current.scancodes[sc];
// }

// /**
//  * @brief Check whether the given key was down previously.
//  *
//  * @param sc Scancode of the key to be checked.
//  *
//  * @return true if was down, else false.
//  */
// b8 inputWasKeyDown(Scancode sc) {
//     sassert_msg(input_state, "Input system is not initialized.");
//     return input_state->keyboard_previous.scancodes[sc];
// }

// /**
//  * @brief Check whether the given key was up previously.
//  *
//  * @param sc Scancode of the key to be checked.
//  *
//  * @return true if was up, else false.
//  */
// b8 inputWasKeyUp(Scancode sc) {
//     sassert_msg(input_state, "Input system is not initialized.");
//     return !input_state->keyboard_previous.scancodes[sc];
// }

/**
 * @brief Process the keyboard events.
 *
 * @param sc Scancode of the key whose state changed
 * @param pressed Whether the key is pressed
 */
void inputProcessKey(Scancode sc, Keycode kc, u32 mod, b8 pressed, b8 repeat) {
    sassert_msg(input_state, "Input system is not initialized.");

    if (repeat) {
        fireEvent(
            EVENT_CODE_KEY_REPEAT, NULL,
            ((EventContext){
                .data.u32 = {[0] = sc, [1] = kc, [2] = mod
                             /*[2] = input_state->keyboard_current.mod*/}
        }));
    }

    // If status changed, then update and fire event
    // TODO: Track the state of the keycode not scancode (or should track both?)
    if (input_state->keyboard_current.scancodes[sc] != pressed) {
        input_state->keyboard_current.scancodes[sc] = pressed;

        // TODO: Logic for detecting left/right mod (if two mods are there)

        fireEvent(
            (pressed ? EVENT_CODE_KEY_PRESS : EVENT_CODE_KEY_RELEASE), NULL,
            (EventContext){
                .data.u32 = {[0] = sc, [1] = kc, [2] = mod
                             /*[2] = input_state->keyboard_current.mod*/}
        });
    }
}

// /**
//  * @brief Check whether the given button is down now.
//  *
//  * @param b Button to be checked.
//  *
//  * @return true if is down, else false.
//  */
// b8 inputIsButtonDown(Button b) {
//     sassert_msg(input_state, "Input system is not initialized.");
//     return input_state->mouse_current.button_state.buttons[b];
// }

// /**
//  * @brief Check whether the given button is up now.
//  *
//  * @param b Button to be checked.
//  *
//  * @return true if is up, else false.
//  */
// b8 inputIsButtonUp(Button b) {
//     sassert_msg(input_state, "Input system is not initialized.");
//     return !input_state->mouse_current.button_state.buttons[b];
// }

// /**
//  * @brief Check whether the given button was down previously.
//  *
//  * @param b Button to be checked.
//  *
//  * @return true if was down, else false.
//  */
// b8 inputWasButtonDown(Button b) {
//     sassert_msg(input_state, "Input system is not initialized.");
//     return input_state->mouse_previous.button_state.buttons[b];
// }

// /**
//  * @brief Check whether the given button was up previously.
//  *
//  * @param b Button to be checked.
//  *
//  * @return true if was up, else false.
//  */
// b8 inputWasButtonUp(Button b) {
//     sassert_msg(input_state, "Input system is not initialized.");
//     return !input_state->mouse_previous.button_state.buttons[b];
// }

// /**
//  * @brief Check whether scrolling towards given direction now.
//  *
//  * @param direction Scroll direction
//  *
//  * @return true if is, else false.
//  */
// b8 inputIsScrolling(Scroll direction) {
//     sassert_msg(input_state, "Input system is not initialized.");
//     return input_state->mouse_current.scroll_state.scrolls[direction];
// }

// /**
//  * @brief Check whether scrolling towards given direction previously.
//  *
//  * @param direction Scroll direction
//  *
//  * @return true if was, else false.
//  */
// b8 inputWasScrolling(Scroll direction) {
//     sassert_msg(input_state, "Input system is not initialized.");
//     return input_state->mouse_previous.scroll_state.scrolls[direction];
// }

/**
 * @brief Process the mouse button events.
 *
 * @param b Button whose state changed
 * @param x x position
 * @param y y position
 * @param mod Modifiers state
 * @param pressed Whether the button is pressed
 */
void inputProcessButton(Button b, f64 x, f64 y, u32 mod, b8 pressed) {
    sassert_msg(input_state, "Input system is not initialized.");

    if (input_state->mouse_current.button_state.buttons[b] != pressed) {
        input_state->mouse_current.button_state.buttons[b] = pressed;

        fireEvent(
            (pressed ? EVENT_CODE_BUTTON_PRESS : EVENT_CODE_BUTTON_RELEASE),
            NULL,
            ((EventContext){
                .data.u32 = {[0] = b, [1] = x, [2] = y, [3] = mod}
        }));
    }
}

/**
 * @brief Process the scroll events.
 *
 * @param direction Scroll direction
 * @param mod Modifiers state
 * @param delta delta :)
 */
void inputProcessScroll(Scroll direction, u32 mod, u32 delta) {
    sassert_msg(input_state, "Input system is not initialized.");

    fireEvent(EVENT_CODE_SCROLL, NULL,
              ((EventContext){
                  .data.u32 = {[0] = direction, [1] = delta, [2] = mod}
    }));
}

/**
 * @brief Process the pointer motion.
 *
 * @param x Current x position
 * @param y Current y position
 */
void inputProcessPointerMotion(f64 x, f64 y) {
    sassert_msg(input_state, "Input system is not initialized.");

    input_state->mouse_current.scroll_state.x = x;
    input_state->mouse_current.scroll_state.y = y;

    fireEvent(EVENT_CODE_POINTER_MOTION, NULL,
              ((EventContext){
                  .data.i32 = {[0] = x, [1] = y}
    }));
}
