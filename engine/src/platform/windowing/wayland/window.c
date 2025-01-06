#include "../../window.h"

#ifdef SPLATFORM_WINDOWING_WAYLAND
// Documentation used
// https://wayland-book.com/
// https://wayland.app/protocols/
// https://gist.github.com/lmarz/1059f7c4101a15e2a04d6991d7b7b3d1
// https://github.com/zezba9000/WaylandClientWindow/blob/master/main.c

// NOTE: The listeners are not copied and we need to manage the life cycle of
// NOTE: listener structs. Make sure to declare them in global or declare them
// NOTE: as static. Spent almost a day trying to figure it out and fix the
// NOTE: listener function for opcode 1 of wl_seat is NULL error!

    #define _GNU_SOURCE
    #include <fcntl.h>
    #include <linux/input-event-codes.h>
    #include <poll.h>
    #include <sys/mman.h>
    #include <unistd.h>
    #include <wayland-client.h>

    #include "core/assertions.h"
    #include "core/logger.h"
    #include "core/memory.h"
    #include "core/sstring.h"
    #include "input/input.h"
    #include "protocols/stable/xdg-shell/xdg-shell.h"
    #include "protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.h"

typedef struct WaylandState {
        struct wl_display *wl_display;
        struct wl_compositor *wl_compositor;
        struct wl_surface *wl_surface;

        struct xdg_surface *xdg_surface;
        struct xdg_wm_base *xdg_wm_base;
        struct xdg_toplevel *xdg_toplevel;

        struct wl_shm *wl_shm;

        struct wl_seat *wl_seat;
        struct wl_pointer *wl_pointer;
        struct wl_keyboard *wl_keyboard;

        struct zxdg_decoration_manager_v1 *zxdg_decoration_manager_v1;
        struct zxdg_toplevel_decoration_v1 *zxdg_toplevel_decoration_v1;

        i32 display_fd;

        c8 *title;
        b8 quit;

        u32 width, height, stride, size;
} WaylandState;

static WaylandState *wayland_state;

void wlRegistryGlobal(void *data, struct wl_registry *wl_registry, u32 name,
                      const c8 *interface, u32 version);
void wlRegistryGlobalRemove(void *data, struct wl_registry *wl_registry,
                            u32 name);

void xdgSurfaceConfigure(void *data, struct xdg_surface *xdg_surface,
                         u32 serial);

void xdgToplevelConfigureBounds(void *data, struct xdg_toplevel *xdg_toplevel,
                                i32 width, i32 height);
void xdgToplevelConfigure(void *data, struct xdg_toplevel *xdg_toplevel,
                          i32 width, i32 height, struct wl_array *states);
void xdgToplevelClose(void *data, struct xdg_toplevel *xdg_toplevel);

void xdgWmBasePing(void *data, struct xdg_wm_base *xdg_wm_base, u32 serial);

void seatCapabilityHandler(void *data, struct wl_seat *wl_seat,
                           u32 capabilities);

void wlPointerMotion(void *data, struct wl_pointer *wl_pointer, u32 time,
                     wl_fixed_t x, wl_fixed_t y);
void wlPointerButton(void *data, struct wl_pointer *wl_pointer, u32 serial,
                     u32 time, u32 button, u32 state);
void wlPointerAxis(void *data, struct wl_pointer *wl_pointer, u32 time,
                   u32 axis, wl_fixed_t value);
void wlPointerAxisDiscrete(void *data, struct wl_pointer *wl_pointer, u32 axis,
                           i32 discrete);
void wlPointerAxisRelativeDirection(void *data, struct wl_pointer *wl_pointer,
                                    u32 axis, u32 direction);
void wlPointerAxisSource(void *data, struct wl_pointer *wl_pointer,
                         u32 axis_source);
void wlPointerAxisStop(void *data, struct wl_pointer *wl_pointer, u32 time,
                       u32 axis);
void wlPointerAxisValue120(void *data, struct wl_pointer *wl_pointer, u32 axis,
                           i32 value120);
void wlPointerEnter(void *data, struct wl_pointer *wl_pointer, u32 serial,
                    struct wl_surface *wl_surface, wl_fixed_t surface_x,
                    wl_fixed_t surface_y);
void wlPointerFrame(void *data, struct wl_pointer *wl_pointer);
void wlPointerLeave(void *data, struct wl_pointer *wl_pointer, u32 serial,
                    struct wl_surface *wl_surface);

void wlKeyboardKey(void *data, struct wl_keyboard *wl_keybord, u32 serial,
                   u32 time, u32 key, u32 state);
void wlKeyboardEnter(void *data, struct wl_keyboard *wl_keyboard, u32 serial,
                     struct wl_surface *wl_surface, struct wl_array *keys);
void wlKeyboardKeymap(void *data, struct wl_keyboard *wl_keyboard, u32 format,
                      i32 fd, u32 size);
void wlKeyboardLeave(void *data, struct wl_keyboard *wl_keyboard, u32 serial,
                     struct wl_surface *wl_surface);
void wlKeyboardModifiers(void *data, struct wl_keyboard *wl_keyboard,
                         u32 serial, u32 mods_depressed, u32 mods_latched,
                         u32 mods_locked, u32 group);
void wlKeyboardRepeatInfo(void *data, struct wl_keyboard *wl_keyboard, i32 rate,
                          i32 delay);

void wlBufferRelease(void *data, struct wl_buffer *wl_buffer);

void zxdgToplevelDecorationV1Configure(
    void *data, struct zxdg_toplevel_decoration_v1 *zxdg_toplevel_decoration_v1,
    enum zxdg_toplevel_decoration_v1_mode zxdg_toplevel_decoration_v1_mode);

struct wl_buffer *createAndDrawWlBuffer();

void seatNameHandler(void *data, struct wl_seat *wl_seat, const c8 *name);

void xdgToplevelWmCapabilities(void *data, struct xdg_toplevel *xdg_toplevel,
                               struct wl_array *capabilities);

void wlShmFormat(void *data, struct wl_shm *wl_shm, u32 format);

/**
 * @brief Implementation for wayland.
 *
 * Call with state NULL to get the size to be allocated and call once again with
 * pointer to the allocated memory to actually initialize.
 *
 * @param size The size of allocation required
 * @param state Pointer to the allocated memory
 * @param config Main Window configuration
 *
 * @return Returns true on initializing successfully, else false.
 */
b8 initializePlatformWindowing(MainWindowConfig *config, u64 *size,
                               void *state) {
    sassert_msg(!wayland_state, "Initializing the Windowing system twice?");

    *size = sizeof(WaylandState);

    if (!state) return false;

    wayland_state = (WaylandState *)state;

    // Zero out before using
    sMemZeroOut(wayland_state, sizeof(WaylandState));

    // TODO: Error handling

    // Connect to wayland display
    wayland_state->wl_display = wl_display_connect(NULL);
    if (!wayland_state->wl_display) {
        sError("Failed to connect to wayland server!");
        return false;
    }

    wayland_state->display_fd = wl_display_get_fd(wayland_state->wl_display);

    struct wl_registry *wl_registry =
        wl_display_get_registry(wayland_state->wl_display);
    if (!wl_registry) {
        sError("Failed to get the registry");
        return false;
    }

    static const struct wl_registry_listener wl_registry_listener = {
        .global = wlRegistryGlobal, .global_remove = wlRegistryGlobalRemove};

    wl_registry_add_listener(wl_registry, &wl_registry_listener, NULL);
    wl_display_roundtrip(wayland_state->wl_display);

    if (!wayland_state->wl_compositor || !wayland_state->xdg_wm_base
        || !wayland_state->wl_seat || !wayland_state->wl_shm) {
        sError("Couldn't register for all the required globals");
        return false;
    }

    wayland_state->wl_surface =
        wl_compositor_create_surface(wayland_state->wl_compositor);
    if (!wayland_state->wl_surface) {
        sError("Failed to get the wl_surface");
        return false;
    }

    wayland_state->xdg_surface = xdg_wm_base_get_xdg_surface(
        wayland_state->xdg_wm_base, wayland_state->wl_surface);
    if (!wayland_state->xdg_surface) {
        sError("Failed to get the xdg_surface");
        return false;
    }

    wayland_state->xdg_toplevel =
        xdg_surface_get_toplevel(wayland_state->xdg_surface);
    if (!wayland_state->xdg_toplevel) {
        sError("Failed to get the xdg_toplevel");
        return false;
    }

    static const struct xdg_surface_listener xdg_surface_listener = {
        .configure = xdgSurfaceConfigure};
    xdg_surface_add_listener(wayland_state->xdg_surface, &xdg_surface_listener,
                             NULL);

    static const struct xdg_toplevel_listener xdg_toplevel_listener = {
        .configure = xdgToplevelConfigure,
        .close = xdgToplevelClose,
        .configure_bounds = xdgToplevelConfigureBounds,
        .wm_capabilities = xdgToplevelWmCapabilities};
    xdg_toplevel_add_listener(wayland_state->xdg_toplevel,
                              &xdg_toplevel_listener, NULL);

    static const struct xdg_wm_base_listener xdg_wm_base_listener = {
        .ping = xdgWmBasePing};
    xdg_wm_base_add_listener(wayland_state->xdg_wm_base, &xdg_wm_base_listener,
                             NULL);

    xdg_toplevel_set_app_id(wayland_state->xdg_toplevel, config->name);

    c8 *app_name = sStringConcatC8(config->name, " - Wayland", 0, 11, NULL);
    if (!platformSetWindowTitle(app_name))
        sError("Couldn't set the window title");
    sFree(app_name);

    wayland_state->width = config->width;
    wayland_state->height = config->height;
    xdg_toplevel_set_min_size(wayland_state->xdg_toplevel, wayland_state->width,
                              wayland_state->height);

    // Window decoration
    if (wayland_state->zxdg_decoration_manager_v1) {
        wayland_state->zxdg_toplevel_decoration_v1 =
            zxdg_decoration_manager_v1_get_toplevel_decoration(
                wayland_state->zxdg_decoration_manager_v1,
                wayland_state->xdg_toplevel);

        static const struct zxdg_toplevel_decoration_v1_listener
            zxdg_toplevel_decoration_v1_listener = {
                .configure = zxdgToplevelDecorationV1Configure};
        zxdg_toplevel_decoration_v1_add_listener(
            wayland_state->zxdg_toplevel_decoration_v1,
            &zxdg_toplevel_decoration_v1_listener, NULL);
    } else {
        sError("Window decoration will not be there");
    }

    // Todo: Make it as a parameter may be
    // Must set the title at least once before showing the window
    if (!platformSetWindowVisible(true)) sError("Couldn't show the window");

    // flush
    wl_display_flush(wayland_state->wl_display);

    return true;
}

/**
 * @brief Implementation of wayland.
 *
 * @param state Pointer to the allocated memory
 */
void shutdownPlatformWindowing(void *state) {
    UNUSED(state);
    sassert_msg(wayland_state,
                "Shutting down windowing system twice or not initialized?");

    if (wayland_state->title) sFree(wayland_state->title);

    if (wayland_state->zxdg_toplevel_decoration_v1)
        zxdg_toplevel_decoration_v1_destroy(
            wayland_state->zxdg_toplevel_decoration_v1);

    if (wayland_state->xdg_toplevel)
        xdg_toplevel_destroy(wayland_state->xdg_toplevel);

    if (wayland_state->xdg_surface)
        xdg_surface_destroy(wayland_state->xdg_surface);

    if (wayland_state->wl_surface)
        wl_surface_destroy(wayland_state->wl_surface);

    if (wayland_state->wl_display)
        wl_display_disconnect(wayland_state->wl_display);
}

/**
 * @brief Loop through all the messages and fire the corresponding events.
 *
 * @return Returns false if application quit was recieved else true.
 */
b8 platformWindowPumpMessages(void) {
    sassert_msg(wayland_state, "Windowing system is not initialized?");

    while (wl_display_prepare_read(wayland_state->wl_display))
        wl_display_dispatch_pending(wayland_state->wl_display);

    wl_display_flush(wayland_state->wl_display);

    struct pollfd pollfd = {.fd = wayland_state->display_fd, .events = POLLIN};
    // NOTE: Since we are doing nothing within the engineRun, for now this is
    // NOTE: blocking (equivalent to calling wl_display_dispatch)
    if (poll(&pollfd, 1, -1) < 1) {  // -1 -> error 0 -> no event
        // sDebug("No event to process");
        wl_display_cancel_read(wayland_state->wl_display);
    } else {
        // sDebug("Event to process");
        wl_display_read_events(wayland_state->wl_display);
    }

    wl_display_dispatch_pending(wayland_state->wl_display);

    return !wayland_state->quit;
}

// while (!wayland_state->quit
//        && wl_display_dispatch(wayland_state->wl_display) > 0);
// while (!wayland_state->quit
//        && wl_display_dispatch_pending(wayland_state->wl_display));
// wl_display_flush(wayland_state->wl_display);
// wl_display_dispatch_pending(wayland_state->wl_display);

b8 platformWindowCreate() {
    sassert_msg(wayland_state, "Windowing system is not initialized?");
    // TODO:
    return false;
}

void platformWindowDestroy() {
    sassert_msg(wayland_state, "Windowing system is not initialized?");
    // TODO:
}

/**
 * @brief Chnage the visibility of the window (wayland implementation).
 *
 * If called with true even if the window is visible, or called with false even
 * if the window is not visible, no error will be generated.
 *
 * @param visibility if true make window visible
 *
 * @return Returns true if changes were made successfully.
 */
b8 platformSetWindowVisible(b8 visible) {
    UNUSED(visible);
    sassert_msg(wayland_state, "Windowing system is not initialized?");

    if (visible) {
        struct wl_buffer *wl_buffer = createAndDrawWlBuffer();
        if (!wl_buffer) {
            sError("Failed to create the buffer");
            return false;
        }
        wl_surface_attach(wayland_state->wl_surface, wl_buffer, 0, 0);
    } else {
        wl_surface_attach(wayland_state->wl_surface, NULL, 0, 0);
    }

    wl_surface_commit(wayland_state->wl_surface);

    return true;
}

/**
 * @brief Set the title of the window (wayland implementation).
 *
 * @param title The title
 *
 * @return Returns true if title was changed successfully.
 */
b8 platformSetWindowTitle(const c8 *title) {
    sassert_msg(wayland_state, "Windowing system is not initialized?");

    // Keep track of the title
    // Before that free the title if it was set
    if (wayland_state->title) sFree(wayland_state->title);
    wayland_state->title = sStringCopyC8(title, 0);

    xdg_toplevel_set_title(wayland_state->xdg_toplevel, title);

    return true;
}

/**
 * @brief Get the title of the window (wayland implementation).
 *
 * @return Returns the malloced stirng, user should call sFree.
 */
c8 *platformGetWindowTitle(void) {
    sassert_msg(wayland_state, "Windowing system is not initialized?");
    return sStringCopyC8(wayland_state->title, 0);
}

/**
 * @brief global registry listener [INTERNAL FUNCTION].
 *
 * @param data
 * @param registry
 * @param name
 * @param interface
 * @param version
 */
void wlRegistryGlobal(void *data, struct wl_registry *wl_registry, u32 name,
                      const c8 *interface, u32 version) {
    UNUSED(data);
    UNUSED(version);
    sassert_msg(wayland_state, "Windowing system is not initialized?");

    // TODO: Version compatibility check
    if (sStringEqualC8(interface, wl_compositor_interface.name, 0)) {
        // sDebug("wl_compositor server version: %d", version);
        wayland_state->wl_compositor = (struct wl_compositor *)wl_registry_bind(
            wl_registry, name, &wl_compositor_interface, 6);
    } else if (sStringEqualC8(interface, xdg_wm_base_interface.name, 0)) {
        // sDebug("xdg_wm_base server version: %d", version);
        wayland_state->xdg_wm_base = (struct xdg_wm_base *)wl_registry_bind(
            wl_registry, name, &xdg_wm_base_interface, 6);
    } else if (sStringEqualC8(interface, wl_shm_interface.name, 0)) {
        // sDebug("wl_shm server version: %d", version);
        wayland_state->wl_shm = (struct wl_shm *)wl_registry_bind(
            wl_registry, name, &wl_shm_interface,
            1);  // even though there is version 2 in the xml, 1 is supported

        static const struct wl_shm_listener wl_shm_listener = {.format =
                                                                   wlShmFormat};
        wl_shm_add_listener(wayland_state->wl_shm, &wl_shm_listener, NULL);
    } else if (sStringEqualC8(interface, wl_seat_interface.name, 0)) {
        // sDebug("wl_seat server version: %d", version);
        wayland_state->wl_seat = (struct wl_seat *)wl_registry_bind(
            wl_registry, name, &wl_seat_interface, 9);
        static const struct wl_seat_listener wl_seat_listener = {
            .capabilities = seatCapabilityHandler, .name = seatNameHandler};
        wl_seat_add_listener(wayland_state->wl_seat, &wl_seat_listener, NULL);
    } else if (sStringEqualC8(interface,
                              zxdg_decoration_manager_v1_interface.name, 0)) {
        // sDebug("zxdg_decoration_manager_v1 server version: %d", version);
        wayland_state->zxdg_decoration_manager_v1 =
            (struct zxdg_decoration_manager_v1 *)wl_registry_bind(
                wl_registry, name, &zxdg_decoration_manager_v1_interface, 1);
    }
}

/**
 * @brief global registry remove listener [INTERNAL FUNCTION]
 *
 * @param data
 * @param wl_registry
 * @param name
 */
void wlRegistryGlobalRemove(void *data, struct wl_registry *wl_registry,
                            u32 name) {
    UNUSED(data);
    UNUSED(wl_registry);
    UNUSED(name);
    sassert_msg(wayland_state, "Windowing system is not initialized?");
}

/**
 * @brief xdg surface configure handler [INTERNAL FUNCTION].
 *
 * @param data
 * @param xdg_surface
 * @param serial
 */
void xdgSurfaceConfigure(void *data, struct xdg_surface *xdg_surface,
                         u32 serial) {
    UNUSED(data);
    xdg_surface_ack_configure(xdg_surface, serial);
    wl_surface_commit(wayland_state->wl_surface);
    // wl_display_flush(wayland_state->wl_display);
}

/**
 * @brief xdg toplevel configure bounds handler [INTERNAL FUNCTION].
 *
 * @param data
 * @param xdg_toplevel
 * @param width
 * @param height
 */
void xdgToplevelConfigureBounds(void *data, struct xdg_toplevel *xdg_toplevel,
                                i32 width, i32 height) {
    UNUSED(data);
    UNUSED(xdg_toplevel);
    UNUSED(width);
    UNUSED(height);
}

/**
 * @brief xdg toplevel configure handler [INTERNAL FUNCTION].
 *
 * @param data
 * @param xdg_toplevel
 * @param width
 * @param height
 * @param states
 */
void xdgToplevelConfigure(void *data, struct xdg_toplevel *xdg_toplevel,
                          i32 width, i32 height, struct wl_array *states) {
    UNUSED(data);
    UNUSED(xdg_toplevel);
    UNUSED(width);
    UNUSED(height);
    UNUSED(states);
}

/**
 * @brief xdg toplevel close handler [INTERNAL FUNCTION].
 *
 * @param data
 * @param xdg_toplevel
 */
void xdgToplevelClose(void *data, struct xdg_toplevel *xdg_toplevel) {
    UNUSED(data);
    UNUSED(xdg_toplevel);
    wayland_state->quit = true;
}

/**
 * @brief Send pong when pinged [INTERNAL FUNCTION].
 *
 * @param data
 * @param xdg_wm_base
 * @param serial
 */
void xdgWmBasePing(void *data, struct xdg_wm_base *xdg_wm_base, u32 serial) {
    UNUSED(data);
    xdg_wm_base_pong(xdg_wm_base, serial);
}

/**
 * @brief Seat capability handler [INTERNAL FUNCTION].
 *
 * @param data
 * @param wl_seat
 * @param capabilities
 */
void seatCapabilityHandler(void *data, struct wl_seat *wl_seat,
                           u32 capabilities) {
    UNUSED(data);
    if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
        wayland_state->wl_pointer = wl_seat_get_pointer(wl_seat);
        static const struct wl_pointer_listener wl_pointer_listener = {
            .axis = wlPointerAxis,
            .axis_discrete = wlPointerAxisDiscrete,
            .axis_relative_direction = wlPointerAxisRelativeDirection,
            .axis_source = wlPointerAxisSource,
            .axis_stop = wlPointerAxisStop,
            .axis_value120 = wlPointerAxisValue120,
            .button = wlPointerButton,
            .enter = wlPointerEnter,
            .frame = wlPointerFrame,
            .leave = wlPointerLeave,
            .motion = wlPointerMotion};
        wl_pointer_add_listener(wayland_state->wl_pointer, &wl_pointer_listener,
                                NULL);
    }

    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
        wayland_state->wl_keyboard = wl_seat_get_keyboard(wl_seat);
        static const struct wl_keyboard_listener wl_keyboard_listener = {
            .enter = wlKeyboardEnter,
            .key = wlKeyboardKey,
            .keymap = wlKeyboardKeymap,
            .leave = wlKeyboardLeave,
            .modifiers = wlKeyboardModifiers,
            .repeat_info = wlKeyboardRepeatInfo};
        wl_keyboard_add_listener(wayland_state->wl_keyboard,
                                 &wl_keyboard_listener, NULL);
    }
}

/**
 * @brief pointer motion handler [INTERNAL FUNCTION].
 *
 * @param data
 * @param wl_pointer
 * @param time
 * @param x
 * @param y
 */
void wlPointerMotion(void *data, struct wl_pointer *wl_pointer, u32 time,
                     wl_fixed_t x, wl_fixed_t y) {
    UNUSED(data);
    UNUSED(wl_pointer);
    UNUSED(time);
    UNUSED(x);
    UNUSED(y);
    // sDebug("Pointer moved to (%f, %f)", wl_fixed_to_double(x),
    //        wl_fixed_to_double(y));
    // TODO: Haven't implemented yet.
    inputProcessPointerMotion(x, y);
}

/**
 * @brief Pointer button handler [INTERNAL FUNCTION].
 *
 * @param data
 * @param pointer
 * @param serial
 * @param time
 * @param button
 * @param state
 */
void wlPointerButton(void *data, struct wl_pointer *wl_pointer, u32 serial,
                     u32 time, u32 button, u32 state) {
    UNUSED(data);
    UNUSED(wl_pointer);
    UNUSED(time);
    UNUSED(serial);
    // sDebug("Pointer button %d %s", button,
    //        state == WL_POINTER_BUTTON_STATE_PRESSED ? "pressed" :
    //        "released");

    Button b;

    // Using the linux/input-event-codes.h
    switch (button) {
        case BTN_LEFT:
            b = BUTTON_LEFT;
            break;
        case BTN_RIGHT:
            b = BUTTON_RIGHT;
            break;
        case BTN_MIDDLE:
            b = BUTTON_MIDDLE;
            break;
        default:
            b = BUTTON_NONE;
            break;
    }

    // TODO: Either have to change how the input processing works or have to
    // keep track of the position and pass it.
    inputProcessButton(b, state == WL_POINTER_BUTTON_STATE_PRESSED);
}

void wlPointerAxis(void *data, struct wl_pointer *wl_pointer, u32 time,
                   u32 axis, wl_fixed_t value) {
    UNUSED(data);
    UNUSED(wl_pointer);
    UNUSED(time);
    UNUSED(axis);
    UNUSED(value);
    // TODO: Similar to the inputProcessButton, need to be changed
    if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
        inputProcessScroll(((value < 0) ? SCROLL_UP : SCROLL_DOWN),
                           ((value < 0) ? -value : value));
    } else if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
        inputProcessScroll(((value < 0) ? SCROLL_LEFT : SCROLL_RIGHT),
                           ((value < 0) ? -value : value));
    }
}

void wlPointerAxisDiscrete(void *data, struct wl_pointer *wl_pointer, u32 axis,
                           i32 discrete) {
    UNUSED(data);
    UNUSED(wl_pointer);
    UNUSED(axis);
    UNUSED(discrete);
}

void wlPointerAxisRelativeDirection(void *data, struct wl_pointer *wl_pointer,
                                    u32 axis, u32 direction) {
    UNUSED(data);
    UNUSED(wl_pointer);
    UNUSED(axis);
    UNUSED(direction);
}

void wlPointerAxisSource(void *data, struct wl_pointer *wl_pointer,
                         u32 axis_source) {
    UNUSED(data);
    UNUSED(wl_pointer);
    UNUSED(axis_source);
}

void wlPointerAxisStop(void *data, struct wl_pointer *wl_pointer, u32 time,
                       u32 axis) {
    UNUSED(data);
    UNUSED(wl_pointer);
    UNUSED(time);
    UNUSED(axis);
}

void wlPointerAxisValue120(void *data, struct wl_pointer *wl_pointer, u32 axis,
                           i32 value120) {
    UNUSED(data);
    UNUSED(wl_pointer);
    UNUSED(axis);
    UNUSED(value120);
}

void wlPointerEnter(void *data, struct wl_pointer *wl_pointer, u32 serial,
                    struct wl_surface *wl_surface, wl_fixed_t surface_x,
                    wl_fixed_t surface_y) {
    UNUSED(data);
    UNUSED(wl_pointer);
    UNUSED(serial);
    UNUSED(wl_surface);
    UNUSED(surface_x);
    UNUSED(surface_y);
}

void wlPointerFrame(void *data, struct wl_pointer *wl_pointer) {
    UNUSED(data);
    UNUSED(wl_pointer);
}

void wlPointerLeave(void *data, struct wl_pointer *wl_pointer, u32 serial,
                    struct wl_surface *wl_surface) {
    UNUSED(data);
    UNUSED(serial);
    UNUSED(wl_pointer);
    UNUSED(wl_surface);
}

/**
 * @brief keyboard key handler [INTERNAL FUNCTION].
 *
 * @param data
 * @param wl_keyboard
 * @param serial
 * @param time
 * @param key
 * @param state
 */
void wlKeyboardKey(void *data, struct wl_keyboard *wl_keyboard, u32 serial,
                   u32 time, u32 key, u32 state) {
    UNUSED(data);
    UNUSED(wl_keyboard);
    UNUSED(serial);
    UNUSED(time);
    // sDebug("Key %d %s", key,
    //        state == WL_KEYBOARD_KEY_STATE_PRESSED ? "pressed" : "released");
    // TODO: Translation
    inputProcessKey(key, KEYCODE_NONE, state == WL_KEYBOARD_KEY_STATE_PRESSED,
                    false);
}

void wlKeyboardEnter(void *data, struct wl_keyboard *wl_keyboard, u32 serial,
                     struct wl_surface *wl_surface, struct wl_array *keys) {
    UNUSED(data);
    UNUSED(wl_keyboard);
    UNUSED(serial);
    UNUSED(wl_surface);
    UNUSED(keys);
}

void wlKeyboardKeymap(void *data, struct wl_keyboard *wl_keyboard, u32 format,
                      i32 fd, u32 size) {
    UNUSED(data);
    UNUSED(wl_keyboard);
    UNUSED(format);
    UNUSED(fd);
    UNUSED(size);
}

void wlKeyboardLeave(void *data, struct wl_keyboard *wl_keyboard, u32 serial,
                     struct wl_surface *wl_surface) {
    UNUSED(data);
    UNUSED(wl_keyboard);
    UNUSED(serial);
    UNUSED(wl_surface);
}

void wlKeyboardModifiers(void *data, struct wl_keyboard *wl_keyboard,
                         u32 serial, u32 mods_depressed, u32 mods_latched,
                         u32 mods_locked, u32 group) {
    UNUSED(data);
    UNUSED(wl_keyboard);
    UNUSED(serial);
    UNUSED(mods_depressed);
    UNUSED(mods_latched);
    UNUSED(mods_locked);
    UNUSED(group);
}

void wlKeyboardRepeatInfo(void *data, struct wl_keyboard *wl_keyboard, i32 rate,
                          i32 delay) {
    UNUSED(data);
    UNUSED(wl_keyboard);
    UNUSED(rate);
    UNUSED(delay);
}

/**
 * @brief Handle the buffer release [INTERNAL FUNCTION].
 *
 * @param data
 * @param wl_buffer
 */
void wlBufferRelease(void *data, struct wl_buffer *wl_buffer) {
    UNUSED(data);
    wl_buffer_destroy(wl_buffer);
}

/**
 * @brief [INTERNAL FUNCTION].
 *
 * @return Returns the wl_buffer after drawing to it.
 */
struct wl_buffer *createAndDrawWlBuffer() {
    wayland_state->stride = wayland_state->width * sizeof(u32);
    wayland_state->size = wayland_state->height * wayland_state->stride;

    i32 fd = memfd_create(wayland_state->title, MFD_CLOEXEC);
    // i32 fd = shm_open(wayland_state->title, O_RDWR | O_CREAT | O_TRUNC,
    // 0600);
    if (fd < 0) {
        sError("Failed to create anonymous file");
        return NULL;
    }

    if (ftruncate(fd, wayland_state->size) < 0) {
        sError("Failed to truncate the file");
        close(fd);
        return NULL;
    }

    u32 *data = mmap(NULL, wayland_state->size, PROT_READ | PROT_WRITE,
                     MAP_SHARED, fd, 0);
    // shm_unlink(wayland_state->title);
    if (data == MAP_FAILED) {
        sError("Failed mmap");
        close(fd);
        return NULL;
    }

    // Draw
    // u32 n = wayland_state->width * wayland_state->height;
    // for (u32 i = 0; i < n; ++i) data[i] = 0xFF000000;
    sMemSet(data, wayland_state->width * wayland_state->height, 0);

    struct wl_shm_pool *wl_shm_pool =
        wl_shm_create_pool(wayland_state->wl_shm, fd, wayland_state->size);
    struct wl_buffer *wl_buffer = wl_shm_pool_create_buffer(
        wl_shm_pool, 0, wayland_state->width, wayland_state->height,
        wayland_state->stride, WL_SHM_FORMAT_XRGB8888);

    static const struct wl_buffer_listener wl_buffer_listener = {
        .release = wlBufferRelease};
    wl_buffer_add_listener(wl_buffer, &wl_buffer_listener, NULL);

    wl_shm_pool_destroy(wl_shm_pool);
    munmap(data, wayland_state->size);
    close(fd);

    return wl_buffer;
}

/**
 * @brief [INTERNAL FUNCTION].
 */
void zxdgToplevelDecorationV1Configure(
    void *data, struct zxdg_toplevel_decoration_v1 *zxdg_toplevel_decoration_v1,
    enum zxdg_toplevel_decoration_v1_mode zxdg_toplevel_decoration_v1_mode) {
    UNUSED(data);
    UNUSED(zxdg_toplevel_decoration_v1);
    switch (zxdg_toplevel_decoration_v1_mode) {
        case ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE:
            sTrace("Client will not be handling the decoration, so no window "
                   "decoration");
            break;
        case ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE:
            sTrace("Server will manage the decoration");
            break;
    }
}

/**
 * @brief [INTERNAL FUNCTION].
 */
void seatNameHandler(void *data, struct wl_seat *wl_seat, const c8 *name) {
    UNUSED(data);
    UNUSED(wl_seat);
    UNUSED(name);
    // sDebug("The seat name is '%s'", name);
}

/**
 * @brief [INTERNAL FUNCTION].
 */
void xdgToplevelWmCapabilities(void *data, struct xdg_toplevel *xdg_toplevel,
                               struct wl_array *capabilities) {
    UNUSED(data);
    UNUSED(xdg_toplevel);

    const u32 *capability = NULL;

    wl_array_for_each(capability, capabilities)
        sTrace("capability: '%u'", *capability);
}

void wlShmFormat(void *data, struct wl_shm *wl_shm, u32 format) {
    UNUSED(data);
    UNUSED(wl_shm);
    UNUSED(format);
}

#endif
