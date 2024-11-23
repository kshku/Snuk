#include "../../../window.h"

#ifdef SPLATFORM_WINDOWING_X11_XCB
// Documentation used
// https://www.x.org/releases/current/doc/libX11/libX11/libX11.html
// https://www.x.org/releases/X11R7.7/doc/libxcb/tutorial/index.html

// Basic Graphics Programming with the Xlib library
// https://ftp.dim13.org/pub/doc/Xlib.pdf

    #include <stdlib.h>
    #include <xcb/xcb.h>

    #include "core/assertions.h"
    #include "core/logger.h"
    #include "core/memory.h"

typedef struct XCBState {
        xcb_connection_t *connection;
        xcb_screen_t *screen;
        xcb_window_t app_window;
        xcb_atom_t wm_delete_window, wm_protocols;
} XCBState;

static XCBState *xcb_state;

/**
 * @brief Implementation for xcb.
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
    sassert_msg(!xcb_state, "Initializing windowing system twice?");

    *size = sizeof(XCBState);

    if (!state) return false;

    xcb_state = (XCBState *)state;

    // TODO: Error handling
    // Open connection to X server
    i32 screen_number;
    xcb_state->connection = xcb_connect(NULL, &screen_number);
    if (xcb_connection_has_error(xcb_state->connection)) {
        sError("Faild to open connection to X server via xcb");
        return false;
    }

    // Getting screen number
    xcb_screen_iterator_t iter =
        xcb_setup_roots_iterator(xcb_get_setup(xcb_state->connection));
    for (i32 i = 0; i < screen_number; ++i) xcb_screen_next(&iter);
    xcb_state->screen = iter.data;

    // Generate id for window
    xcb_state->app_window = xcb_generate_id(xcb_state->connection);

    // Similar to XSetWindowAttributes
    u32 value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    u32 event_mask = XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE
                   | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE
                   | XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_EXPOSURE
                   | XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    u32 value_list[2] = {xcb_state->screen->black_pixel, event_mask};

    // Create window
    xcb_void_cookie_t window_coockie = xcb_create_window(
        xcb_state->connection, XCB_COPY_FROM_PARENT, xcb_state->app_window,
        xcb_state->screen->root, config->x, config->y, config->width,
        config->height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
        xcb_state->screen->root_visual, value_mask, value_list);
    UNUSED(window_coockie);

    // Atoms
    xcb_intern_atom_cookie_t wm_delete_window_cookie =
        xcb_intern_atom(xcb_state->connection, false, 16, "WM_DELETE_WINDOW");

    xcb_intern_atom_cookie_t wm_protocols_cookie =
        xcb_intern_atom(xcb_state->connection, false, 12, "WM_PROTOCOLS");

    xcb_intern_atom_reply_t *wm_delete_window_reply = xcb_intern_atom_reply(
        xcb_state->connection, wm_delete_window_cookie, NULL);
    xcb_intern_atom_reply_t *wm_protocols_reply =
        xcb_intern_atom_reply(xcb_state->connection, wm_protocols_cookie, NULL);

    xcb_state->wm_delete_window = wm_delete_window_reply->atom;
    xcb_state->wm_protocols = wm_protocols_reply->atom;

    free(wm_delete_window_reply);
    free(wm_protocols_reply);

    // Get notified when the window is getting destroyed
    xcb_change_property(xcb_state->connection, XCB_PROP_MODE_REPLACE,
                        xcb_state->app_window, xcb_state->wm_protocols,
                        XCB_ATOM_ATOM, 32, 1, &xcb_state->wm_delete_window);

    // Set class hint
    // TODO: Don't know how

    if (!platformSetWindowTitle(config->name))
        sError("Couldn't set the window title");

    // Todo: Make it as a parameter may be
    if (!platformSetWindowVisible(true)) sError("Couldn't show the window");

    // Make sure to flush so that everything will be sent to the server
    if (xcb_flush(xcb_state->connection) <= 0) {
        sError("Couldn't flush to X server");
        return false;
    }

    return true;
}

/**
 * @brief Implementation of xlib.
 *
 * @param state Pointer to the allocated memory
 */
void shutdownPlatformWindowing(void *state) {
    sassert_msg(xcb_state,
                "Shutting down windowing system twice or not initialized?");
    UNUSED(state);

    xcb_destroy_window(xcb_state->connection, xcb_state->app_window);
    xcb_disconnect(xcb_state->connection);
}

/**
 * @brief Loop through all the messages and fire the corresponding events.
 *
 * @return Returns false if application quit was recieved else true.
 */
b8 platformWindowPumpMessages(void) {
    sassert_msg(xcb_state, "Windowing system is not initialized?");

    b8 quit = false;

    xcb_generic_event_t *event;

    // xcb_wait_for_event (blocking) is similar to XNextEvent
    // So use xcb_poll_for_event(non-blocking)
    // ? Do I need to check for !quit
    while (!quit && (event = xcb_poll_for_event(xcb_state->connection))) {
        // should be & with ~0x80. Don't know more
        switch (event->response_type & ~0x80) {
            case XCB_KEY_PRESS:
                // TODO:
                break;
            case XCB_KEY_RELEASE:
                // TODO:
                break;
            case XCB_BUTTON_PRESS:
                // TODO:
                break;
            case XCB_BUTTON_RELEASE:
                // TODO:
                break;
            case XCB_MOTION_NOTIFY:
                // TODO:
                break;
            case XCB_EXPOSE:
                // TODO:
                break;
            case XCB_CONFIGURE_NOTIFY:
                // TODO:
                break;
            case XCB_CLIENT_MESSAGE:
                if (((xcb_client_message_event_t *)event)->data.data32[0]
                    == xcb_state->wm_delete_window)
                    quit = true;
                break;
            default:
                sTrace("An event is being ignored: Event type: %d",
                       (event->response_type & ~0x80));
                break;
        }
        free(event);
    }

    return !quit;
}

b8 platformWindowCreate() {
    // TODO:
    return true;
}

void platformWindowDestroy() {
    // Todo:
}

/**
 * @brief Chnage the visibility of the window (xlib implementation).
 *
 * If called with true even if the window is visible, or called with false even
 * if the window is not visible, no error will be generated.
 *
 * @param visibility if true make window visible
 *
 * @return Returns true if changes were made successfully.
 */
b8 platformSetWindowVisible(b8 visible) {
    sassert_msg(xcb_state, "Windowing system is not initialized?");

    if (visible) xcb_map_window(xcb_state->connection, xcb_state->app_window);
    else xcb_unmap_window(xcb_state->connection, xcb_state->app_window);
    return true;
}

/**
 * @brief Set the title of the window (xlib implementation).
 *
 * @param title The title
 *
 * @return Returns true if title was changed successfully.
 */
b8 platformSetWindowTitle(const char *title) {
    sassert_msg(xcb_state, "Windowing system is not initialized?");

    u32 title_length;
    for (title_length = 0; title[title_length]; ++title_length);

    xcb_change_property(xcb_state->connection, XCB_PROP_MODE_REPLACE,
                        xcb_state->app_window, XCB_ATOM_WM_NAME,
                        XCB_ATOM_STRING, 8, title_length, title);
    return true;
}

/**
 * @brief Get the title of the window (xlib implementation).
 *
 * @param[out] title Title will be copied to this
 * @param size Maximum size can be written to the title
 *
 * @return Returns true if title was set successfully, else false.
 */
b8 platformGetWindowTitle(char *title, u64 size) {
    UNUSED(title);
    UNUSED(size);
    sassert_msg(xcb_state, "Windowing system is not initialized?");

    // Todo: Don't know
    return false;
}

#endif
