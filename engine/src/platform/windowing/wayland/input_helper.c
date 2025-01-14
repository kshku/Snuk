#include "input_helper.h"

#ifdef SPLATFORM_WINDOWING_WAYLAND

static f64 current_x, current_y;

void pointerEventUpdatePosition(f64 x, f64 y) {
    current_x = x;
    current_y = y;
}

void pointerEventGetPosition(f64 *x, f64 *y) {
    *x = current_x;
    *y = current_y;
}

f64 pointerEventGetPositionX(void) {
    return (u32)current_x;
}

f64 pointerEventGetPositionY(void) {
    return (u32)current_y;
}

#endif
