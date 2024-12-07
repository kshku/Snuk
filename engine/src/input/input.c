#include "input.h"

#include "core/assertions.h"
#include "core/event.h"
#include "core/memory.h"

typedef struct InputState {
        KeyboardState keyboard_current;
        KeyboardState keyboard_previous;
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
 *
 * @param state Pointer to the allocated memory for the input system
 */
void shutdownInput(void *state) {
    UNUSED(state);
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
}

/**
 * @brief Check whether the given key is down now.
 *
 * @param sc scancode of the key to be checked.
 *
 * @return true if is down, else false.
 */
b8 inputIsKeyDown(ScanCode sc) {
    sassert_msg(input_state, "Input system is not initialized.");
    return input_state->keyboard_current.scancodes[sc];
}

/**
 * @brief Check whether the given key is up now.
 *
 * @param sc scancode of the key to be checked.
 *
 * @return true if is up, else false.
 */
b8 inputIsKeyUp(ScanCode sc) {
    sassert_msg(input_state, "Input system is not initialized.");
    return !input_state->keyboard_current.scancodes[sc];
}

/**
 * @brief Check whether the given key was down previously.
 *
 * @param sc scancode of the key to be checked.
 *
 * @return true if was down, else false.
 */
b8 inputWasKeyDown(ScanCode sc) {
    sassert_msg(input_state, "Input system is not initialized.");
    return input_state->keyboard_previous.scancodes[sc];
}

/**
 * @brief Check whether the given key was up previously.
 *
 * @param sc scancode of the key to be checked.
 *
 * @return true if was up, else false.
 */
b8 inputWasKeyUp(ScanCode sc) {
    sassert_msg(input_state, "Input system is not initialized.");
    return !input_state->keyboard_previous.scancodes[sc];
}

/**
 * @brief Process the keyboard events.
 *
 * @param sc Scancode of the key whose state changed
 * @param pressed Whether the key is pressed
 */
void inputProcessKey(ScanCode sc, b8 pressed) {
    sassert_msg(input_state, "Input system is not initialized.");

    // If status changed, then update and fire event
    if (input_state->keyboard_current.scancodes[sc] != pressed) {
        input_state->keyboard_current.scancodes[sc] = pressed;

        // TODO: The mod keys should be passed from platform layer or handled
        // TODO: here?
        // switch (sc) {
        //     case SCANCODE_CAPS_LOCK:
        //         break;
        //     case SCANCODE_SCROLL_LOCK:
        //         break;
        //     case SCANCODE_NUM_LOCK:
        //         break;
        //     case SCANCODE_LEFT_CONTROL:
        //         break;
        //     case SCANCODE_LEFT_ALT:
        //         break;
        //     case SCANCODE_LEFT_SHIFT:
        //         break;
        //     case SCANCODE_LEFT_GUI:
        //         break;
        //     case SCANCODE_RIGHT_CONTROL:
        //         break;
        //     case SCANCODE_RIGHT_ALT:
        //         break;
        //     case SCANCODE_RIGHT_SHIFT:
        //         break;
        //     case SCANCODE_RIGHT_GUI:
        //         break;
        //     default:
        //         break;
        // }

        fireEvent((pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED),
                  NULL,
                  (EventContext){
                      .data.i32 = {[0] = sc,
                                   [1] = scanCodeToKeyCode(
                                       sc, input_state->keyboard_current.mod),
                                   [2] = input_state->keyboard_current.mod}
        });
    }
}
