#include "event.h"

#include "assertions.h"
#include "ds/darray.h"
#include "logger.h"
#include "memory.h"

#define EVENT_CODE_MAX 16384

typedef struct RegisterdListener {
        fpEventCallback callback;
        void *listener;
} RegisterdListener;

typedef struct EventState {
        RegisterdListener *entries[EVENT_CODE_MAX];
} EventState;

static EventState *event_state;

/**
 * @brief Initialize the Event system.
 *
 * Call with state NULL to get the size to be allocated and call once again with
 * pointer to the allocated memory to actually initialize.
 *
 * @param size The size of allocation required by Event system
 * @param state Pointer to the allocated memory of the Event system
 *
 * @return Returns true if was initialized successfully.
 */
b8 initializeEvent(u64 *size, void *state) {
    sassert_msg(!event_state, "Initializing the evnet system more than once?");

    *size = sizeof(EventState);

    if (!state) return false;

    event_state = state;

    sZeroOutMem(event_state->entries, EVENT_CODE_MAX);

    return true;
}

/**
 * @brief Shutdown the Event sysetem.
 *
 * @param state Pointer to the allocated memory of the event system.
 */
void shutdownEvent(void *state) {
    UNUSED(state);

    sassert_msg(
        event_state,
        "Shutting down event system more than once or without initializing?");

    for (u32 i = 0; i < EVENT_CODE_MAX; ++i)
        if (event_state->entries[i]) darrayDestroy(event_state->entries[i]);

    event_state = NULL;
}

/**
 * @brief Register for an event.
 *
 * @param code The evnet code
 * @param listener The pointer to the listener instance(can be NULL), which
 * will be passed to the function when event is fired
 * @param callback Function to be called when the event is fired
 *
 * @return Returns true if the rigistration was successufull, else false
 */
SAPI b8 registerEventListener(u16 code, void *listener,
                              fpEventCallback callback) {
    sassert_msg(event_state,
                "Event system is not initialized, cannot register to an event");

    if (!event_state->entries[code])
        event_state->entries[code] = darrayCreate(RegisterdListener);

    u64 listeners_length = darrayLength(event_state->entries[code]);

    for (u32 i = 0; i < listeners_length; ++i) {
        if (event_state->entries[code][i].listener == listener
            && event_state->entries[code][i].callback == callback) {
            sWarn("Trying to register same listener and callback function to "
                  "same event");
            return false;
        }
    }

    darrayPush(
        event_state->entries[code],
        ((RegisterdListener){.listener = listener, .callback = callback}));

    return true;
}

/**
 * @brief Unregister from an event.
 *
 * @param code The event code
 * @param listener The pointer to the listener which was passed when
 * registering
 * @param callback The function to be unregistered
 *
 * @return Returns true if unregistered successfully. If no match found
 * returns false.
 */
SAPI b8 unregisterEventListener(u16 code, void *listener,
                                fpEventCallback callback) {
    sassert_msg(
        event_state,
        "Event system is not initialized, cannot unregister from an event");

    if (!event_state->entries[code]) {
        sWarn("No event was registered for the event code, but unregister "
              "was "
              "called");
        return false;
    }

    u64 listeners_length = darrayLength(event_state->entries[code]);

    for (u32 i = 0; i < listeners_length; ++i) {
        if (event_state->entries[code]->listener == listener
            && event_state->entries[code]->callback == callback) {
            darrayPopAt(event_state->entries[code], i, NULL);
            return true;
        }
    }

    return false;
}

/**
 * @brief Fire an event.
 *
 * Passes the event context to all the listeners either until the event is
 * handled or there are no other listeners
 *
 * @param code The event code
 * @param sender The pointer to sender(can be NULL), which will be passed to
 * the callback functions
 * @param context The event context
 *
 * @return Returns true if the event was handled, else false.
 */
SAPI b8 fireEvent(u16 code, void *sender, EventContext context) {
    sassert_msg(event_state,
                "Event system is not initialized, cannot fire events");

    if (!event_state->entries[code]) {
        sTrace("No listeners were listening to the event code %d", code);
        return false;
    }

    u64 listeners_length = darrayLength(event_state->entries[code]);

    for (u32 i = 0; i < listeners_length; ++i) {
        RegisterdListener listerner = event_state->entries[code][i];
        if (listerner.callback(code, sender, listerner.listener, context)) {
            sTrace("The fired evnet with code %d was handled by listener "
                   "number %d%s listener out of %d %s",
                   code, i + 1,
                   ((i + 1 == 1)   ? "st"
                    : (i + 1 == 2) ? "nd"
                    : (i + 1 == 3) ? "rd"
                                   : "th"),
                   listeners_length,
                   ((listeners_length > 1 ? "listeners" : "listener")));
            return true;
        }
    }

    return false;
}
