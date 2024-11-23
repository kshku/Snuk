#include "../../../window.h"

#ifdef SPLATFORM_WINDOWING_X11_XLIB
// Documentation used
// https://www.x.org/releases/current/doc/libX11/libX11/libX11.html

// Basic Graphics Programming with the Xlib library
// https://ftp.dim13.org/pub/doc/Xlib.pdf

    #include <X11/Xatom.h>
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>

    #include "core/assertions.h"
    #include "core/logger.h"
    #include "core/memory.h"

typedef struct XlibState {
        Display *display;
        i32 screen, screen_width, screen_height;
        Window root_window, app_window;
        u64 white_pixel, black_pixel;
        int (*xlib_error_handler)(Display *, XErrorEvent *);
        Atom wm_delete_window;
} XlibState;

static XlibState *xlib_state;

/**
 * @brief The error handler [INTERNAL FUNCTION]
 *
 * Handle errors or send them to the default error handler.
 *
 * @param display Pointer to the display
 * @param e The Error Event from X
 *
 * @return Value is ignored (The documentation says).
 */
int handleXErrors(Display *display, XErrorEvent *e) {
    char buf[256];  // 256 might be sufficient
    XGetErrorText(display, e->error_code, buf, 256);
    sError("Error from X Server: Request Code: %d, Error Code: %d",
           e->request_code, e->error_code);
    sError("Error description from the X server: %s", buf);
    sInfo("Sending the error to the default handler which might make the "
          "application quit...");

    return xlib_state->xlib_error_handler(display, e);
}

/**
 * @brief Implementation for xlib.
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
    sassert_msg(!xlib_state, "Initializing the Windowing system twice?");

    *size = sizeof(XlibState);

    if (!state) return false;

    xlib_state = state;

    // Open Connection to X server
    xlib_state->display = XOpenDisplay(NULL);
    if (!xlib_state->display) {
        sError("Faild to open connection to X server");
        return false;
    }

    // Set the error handler
    xlib_state->xlib_error_handler = XSetErrorHandler(handleXErrors);

    // Get the default screen number
    xlib_state->screen = DefaultScreen(xlib_state->display);

    // Get width and height of the screen
    xlib_state->screen_width =
        DisplayWidth(xlib_state->display, xlib_state->screen);
    xlib_state->screen_height =
        DisplayHeight(xlib_state->display, xlib_state->screen);

    // Get the root window
    xlib_state->root_window =
        RootWindow(xlib_state->display, xlib_state->screen);

    // Get white pixel and black pixel
    xlib_state->white_pixel =
        WhitePixel(xlib_state->display, xlib_state->screen);
    xlib_state->black_pixel =
        BlackPixel(xlib_state->display, xlib_state->screen);

    // Note: Just for now to test things work
    // TODO: Handling errors

    // Set window attributes
    XSetWindowAttributes attribs;
    u32 attrib_mask = CWEventMask | CWBackPixel;
    attribs.background_pixel = xlib_state->black_pixel;
    attribs.event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask
                       | ButtonReleaseMask | PointerMotionMask | ExposureMask
                       | StructureNotifyMask;

    // Create the main window
    xlib_state->app_window = XCreateWindow(
        xlib_state->display, xlib_state->root_window, config->x, config->y,
        config->width, config->height, 0, CopyFromParent, InputOutput,
        CopyFromParent, attrib_mask, &attribs);

    // Atoms
    xlib_state->wm_delete_window =
        XInternAtom(xlib_state->display, "WM_DELETE_WINDOW", false);

    // Get notified when the window is getting destroyed
    Atom wm_protocol_atoms[1] = {xlib_state->wm_delete_window};
    XSetWMProtocols(xlib_state->display, xlib_state->app_window,
                    wm_protocol_atoms, 1);

    // Set class hint
    XClassHint class_hint = {.res_name = (char *)config->name,
                             .res_class = (char *)config->name};
    XSetClassHint(xlib_state->display, xlib_state->app_window, &class_hint);

    platformSetWindowTitle(config->name);

    // Todo: Make it as a parameter may be
    platformSetWindowVisible(true);

    // Make sure to flush so that everything will be sent to the server
    XFlush(xlib_state->display);

    return true;
}

void shutdownPlatformWindowing(void *state) {
    UNUSED(state);
    XDestroyWindow(xlib_state->display, xlib_state->app_window);
    XCloseDisplay(xlib_state->display);
}

/**
 * @brief Loop through all the messages and fire the corresponding events.
 *
 * @return Returns false if application quit was recieved else true.
 */
b8 platformWindowPumpMessages(void) {
    b8 quit = false;

    XEvent event;

    // XPending is equivalent to XEventsQueued(display, QueuedAfterFlush)
    // XNextEvent is blocking which means if direcly used then this function
    // will not return with true, i.e., utill the application recieves the quit
    // signal the loop will not stop. So use XPending
    while (!quit && XPending(xlib_state->display)) {
        XNextEvent(xlib_state->display, &event);
        switch (event.type) {
            case KeyPress:
                // TODO:
                break;
            case KeyRelease:
                // TODO:
                break;
            case ButtonPress:
                // TODO:
                break;
            case ButtonRelease:
                // TODO:
                break;
            case MotionNotify:
                // TODO:
                break;
            case Expose:
                // TODO:
                break;
            case ConfigureNotify:
                // TODO:
                break;
            case ClientMessage:
                if (event.xclient.data.l[0] == xlib_state->wm_delete_window)
                    quit = true;
                break;
            default:
                sTrace("An event is being ignored: Event type: %d", event.type);
                break;
        }
    }

    return !quit;
}

b8 platformWindowCreate() {
    // TODO:
    return false;
}

void platformWindowDestroy() {
    // TODO:
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
    if (visible) XMapWindow(xlib_state->display, xlib_state->app_window);
    else XUnmapWindow(xlib_state->display, xlib_state->app_window);
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
    XStoreName(xlib_state->display, xlib_state->app_window, title);
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
    char *ret;
    if (XFetchName(xlib_state->display, xlib_state->app_window, &ret)) {
        sMemCopy(title, ret, size);
        XFree(ret);
        return true;
    }

    return false;
}

#endif
