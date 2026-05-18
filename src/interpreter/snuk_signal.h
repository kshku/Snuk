#pragma once

#include "defines.h"

/**
 * @brief Control-flow signal flags raised during item or expression evaluation.
 *
 * The flag values are bit positions so they can be combined into capture and
 * propagate masks (see SNUK_SIGNAL_ALL). Loops and blocks pass these masks to
 * execute_block_expr to decide which signals stop the current frame and which
 * bubble up to an enclosing frame.
 */
typedef enum SnukSignal {
    SNUK_SIGNAL_NONE = 0,
    SNUK_SIGNAL_CONTINUE = 1 << 0,
    SNUK_SIGNAL_BREAK = 1 << 1,
    SNUK_SIGNAL_RETURN = 1 << 2,

    SNUK_SIGNAL_ALL = SNUK_SIGNAL_CONTINUE | SNUK_SIGNAL_BREAK | SNUK_SIGNAL_RETURN,
} SnukSignal;
