#include "event.h"

#include "ds/darray.h"
#include "logger.h"
#include "memory.h"

#define EVENT_CODE_MAX 16384

typedef struct RegisterdListener {
        fpEventCallback callback;
        void *listener;
} RegisterdListener;

typedef struct EventEntry {
        RegisterdListener *listeners;
} EventEntry;

typedef struct EventState {
        EventEntry entry[EVENT_CODE_MAX];
        b8 initialized;
} EventState;

static EventState event_state;

/**
 * @brief Initialize the Event system.
 *
 * @return Returns true if was initialized successfully.
 */
b8 initializeEvent(void) {
    if (event_state.initialized) {
        sError("Event system was already initialized, but called "
               "initializeEvent again");
        return false;
    }

    sZeroOutMem(event_state.entry, EVENT_CODE_MAX);

    event_state.initialized = true;

    return true;
}

/**
 * @brief Shutdown the Event sysetem.
 */
void shutdownEvent(void) {
    if (!event_state.initialized) {
        sError("Event system was not initialized, but shutdownEvnet was "
               "called");
        return;
    }

    for (u32 i = 0; i < EVENT_CODE_MAX; ++i) {
        if (event_state.entry[i].listeners) {
            darrayDestroy(event_state.entry[i].listeners);
            event_state.entry[i].listeners = NULL;
        }
    }

    event_state.initialized = false;
}

/**
 * @brief Register for an event.
 *
 * @param code The evnet code
 * @param listener The pointer to the listener instance(can be NULL), which will
 * be passed to the function when event is fired
 * @param callback Function to be called when the event is fired
 *
 * @return Returns true if the rigistration was successufull, else false
 */
SAPI b8 registerEventListener(u16 code, void *listener,
                              fpEventCallback callback) {
    if (!event_state.initialized) {
        sError("Trying to register to the event without initializing the event "
               "system");
        return false;
    }

    if (!event_state.entry[code].listeners)
        event_state.entry[code].listeners = darrayCreate(RegisterdListener);

    u64 listeners_length = darrayLength(event_state.entry[code].listeners);

    for (u32 i = 0; i < listeners_length; ++i) {
        if (event_state.entry[code].listeners[i].listener == listener
            && event_state.entry[code].listeners[i].callback == callback) {
            sWarn("Trying to register same listener and callback function to "
                  "same event");
            return false;
        }
    }

    darrayPush(
        event_state.entry[code].listeners,
        ((RegisterdListener){.listener = listener, .callback = callback}));

    return true;
}

/**
 * @brief Unregister from an event.
 *
 * @param code The event code
 * @param listener The pointer to the listener which was passed when registering
 * @param callback The function to be unregistered
 *
 * @return Returns true if unregistered successfully. If no match found returns
 * false.
 */
SAPI b8 unregisterEventListener(u16 code, void *listener,
                                fpEventCallback callback) {
    if (!event_state.initialized) {
        sError("Trying to unregister from an event without initializing the "
               "event system");
        return false;
    }

    if (!event_state.entry[code].listeners) {
        sWarn("No event was registered for the event code, but unregister was "
              "called");
        return false;
    }

    u64 listeners_length = darrayLength(event_state.entry[code].listeners);

    for (u32 i = 0; i < listeners_length; ++i) {
        if (event_state.entry[code].listeners[i].listener == listener
            && event_state.entry[code].listeners->callback == callback) {
            darrayPopAt(event_state.entry[code].listeners, i, NULL);
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
 * @param sender The pointer to sender(can be NULL), which will be passed to the
 * callback functions
 * @param context The event context
 *
 * @return Returns true if the event was handled, else false.
 */
SAPI b8 fireEvent(u16 code, void *sender, EventContext context) {
    if (!event_state.initialized) {
        sError(
            "Trying to fire and event without initializing the event system");
        return false;
    }

    if (!event_state.entry[code].listeners) {
        sTrace("No listeners were listening to the event code %d", code);
        return false;
    }

    u64 listeners_length = darrayLength(event_state.entry[code].listeners);

    for (u32 i = 0; i < listeners_length; ++i) {
        RegisterdListener listerner = event_state.entry[code].listeners[i];
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
