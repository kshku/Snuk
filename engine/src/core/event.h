#pragma once

#include "defines.h"

typedef struct EventContext {
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
                        void *ptr;
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
 */
typedef b8 (*fpEventCallback)(u16 code, void *sender, void *listener,
                              EventContext context);

b8 initializeEvent(u64 *size, void *state);

void shutdownEvent(void *state);

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

    /**  Key press and release events.
     * Use data.i32
     * [0] = scancode [1] = keycode [2] = keymod
     */
    DEFINE_SYSTEM_EVENT_CODE(KEY_PRESSED, 0x02),
    DEFINE_SYSTEM_EVENT_CODE(KEY_RELEASED, 0x03),

    EVENT_CODE_MAX_SYSTEM_CODE = 0xff
} SystemEventCode;
