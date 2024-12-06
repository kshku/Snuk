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

typedef enum SystemEventCode {
    // Shutting down the application
    EVENT_CODE_APPLICATION_QUIT = 0x01,

    EVENT_CODE_MAX_SYSTEM_CODE = 0xff
}

SystemEventCode;
