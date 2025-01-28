#pragma once

#include "defines.h"

typedef struct EventContext {
        // Anonymous unions/structs are from c11
        union {
                i64 i64[2];
                u64 u64[2];
                f64 f64[2];

                i32 i32[4];
                u32 u32[4];
                f32 f32[4];

                i16 i16[8];
                u16 u16[8];

                i8 i8[16];
                u8 u8[16];
                b8 b8[16];

                struct {
                        u64 size;
                        void *ptr;  // Free should be called by the handler
                } custom;
        } data;
} EventContext;

/**
 * @brief Callback function pointer.
 *
 * Function signature for the callback funtion
 *
 * @param code The event code
 * @param sender Pointer to the sender
 * @param listener Pointer to the listener
 * @param context Event context data
 *
 * @return Returns ture if the event was handled, else false.
 *
 * @note In case the event is handled and the context is custom data (which is
 * malloced), then the pointer should be freed by the handler.
 */
typedef b8 (*fpEventCallback)(u16 code, void *sender, void *listener,
                              EventContext context);

b8 initializeEvent(u64 *restrict size, void *state);

void shutdownEvent(void);

SAPI b8 registerEventListener(u16 code, void *listener,
                              fpEventCallback callback);

SAPI b8 unregisterEventListener(u16 code, void *listener,
                                fpEventCallback callback);

SAPI b8 fireEvent(u16 code, void *sender, EventContext context);

#define SYSTEM_EVENT_CODE_PREFIX EVENT_CODE_
#define DEFINE_SYSTEM_EVENT_CODE(event, value) \
    CONCAT_EXPANDED(SYSTEM_EVENT_CODE_PREFIX, event) = value

typedef enum SystemEventCode {
    DEFINE_SYSTEM_EVENT_CODE(NONE, 0x00),

    /* Shutting down the application */
    DEFINE_SYSTEM_EVENT_CODE(APPLICATION_QUIT, 0x01),

    /**
     * Key press and release events.
     * Use data.u32
     * [0] = scancode, [1] = keycode, [2] = keymod
     */
    DEFINE_SYSTEM_EVENT_CODE(KEY_PRESS, 0x02),
    DEFINE_SYSTEM_EVENT_CODE(KEY_RELEASE, 0x03),
    DEFINE_SYSTEM_EVENT_CODE(KEY_REPEAT, 0x04),

    /**
     * Button press and release events.
     * Use data.u32
     * [0] = button, [1] = x, [2] = y, [3] = keymod
     */
    DEFINE_SYSTEM_EVENT_CODE(BUTTON_PRESS, 0x05),
    DEFINE_SYSTEM_EVENT_CODE(BUTTON_RELEASE, 0x06),

    /**
     * Scroll events.
     * Use data.u32
     * [0] = direction, [1] = delta, [2] = keymod
     */
    DEFINE_SYSTEM_EVENT_CODE(SCROLL, 0x07),

    /**
     * Pointer motion event.
     * Use data.i32
     * [0] = x, [1] = y
     */
    DEFINE_SYSTEM_EVENT_CODE(POINTER_MOTION, 0x08),

    EVENT_CODE_MAX_SYSTEM_CODE = 0xff
} SystemEventCode;
