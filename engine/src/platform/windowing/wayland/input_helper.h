#pragma once

#include "../../window.h"
#include "defines.h"

#ifdef SPLATFORM_WINDOWING_WAYLAND

    #include "input/mouse/mouse.h"

typedef enum PointerEventType {
    POINTER_EVENT_TYPE_AXIS = BITFLAG(0),
    // Deprecated since version 8 of the wl_seat
    // POINTER_EVENT_TYPE_AXIS_DISCRETE = BITFLAG(1),
    POINTER_EVENT_TYPE_AXIS_RELATIVE_DIRECTION = BITFLAG(2),
    POINTER_EVENT_TYPE_AXIS_SOURCE = BITFLAG(3),
    POINTER_EVENT_TYPE_AXIS_STOP = BITFLAG(4),
    POINTER_EVENT_TYPE_AXIS_VALUE120 = BITFLAG(5),
    POINTER_EVENT_TYPE_BUTTON = BITFLAG(6),
    POINTER_EVENT_TYPE_ENTER = BITFLAG(7),
    POINTER_EVENT_TYPE_LEAVE = BITFLAG(8),
    POINTER_EVENT_TYPE_MOTION = BITFLAG(9),

    POINTER_EVENT_TYPE_AXIS_EVENTS =
        (POINTER_EVENT_TYPE_AXIS | POINTER_EVENT_TYPE_AXIS_SOURCE
         | POINTER_EVENT_TYPE_AXIS_STOP
         | POINTER_EVENT_TYPE_AXIS_RELATIVE_DIRECTION
         | POINTER_EVENT_TYPE_AXIS_VALUE120)
} PointerEventType;

typedef struct PointerEvent {
        u32 event_mask;
        f64 motion_surface_x, motion_surface_y;
        u32 button;
        b8 pressed;
        f64 enter_surface_x, enter_surface_y;

        struct {
                b8 valid;
                f64 value;
                // Deprecated thing
                // i32 discrete;
                u32 direction;
                i32 value120;
        } axes[2];
} PointerEvent;

void pointerEventUpdatePosition(f64 x, f64 y);

void pointerEventGetPosition(f64 *x, f64 *y);

f64 pointerEventGetPositionX(void);

f64 pointerEventGetPositionY(void);

#endif
