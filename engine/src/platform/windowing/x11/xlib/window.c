#include "../../../window.h"

#ifdef SPLATFORM_WINDOWING_X11_XLIB
// Documentation used
// https://www.x.org/releases/current/doc/libX11/libX11/libX11.html

// Basic Graphics Programming with the Xlib library
// https://ftp.dim13.org/pub/doc/Xlib.pdf

    #include <X11/XKBlib.h>
    #include <X11/Xatom.h>
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
    #include <X11/extensions/XInput2.h>

    #include "../../linux_input_helper.h"
    #include "core/assertions.h"
    #include "core/event.h"
    #include "core/logger.h"
    #include "core/memory/memory.h"
    #include "core/sstring.h"
    #include "input/input.h"
    #include "input_helper.h"

typedef struct XlibState {
        Display *display;
        i32 screen, screen_width, screen_height;
        i32 xi_opcode, xi_event_code, xi_error_code;
        i32 xkb_opcode, xkb_event_code, xkb_error_code;
        Window root_window, app_window;
        u64 white_pixel, black_pixel;
        i32 (*xlib_error_handler)(Display *, XErrorEvent *);
        Atom wm_delete_window;

        Scancode xKeyCode_to_Scancode[256];
        XkbDescPtr xkb_desc;
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
i32 handleXErrors(Display *display, XErrorEvent *e) {
    c8 buf[256];  // 256 might be sufficient
    XGetErrorText(display, e->error_code, buf, 256);
    sError("Error from X Server: Request Code: %d, Error Code: %d",
           e->request_code, e->error_code);
    sError("Error description from the X server: %s", buf);
    sInfo("Sending the error to the default handler which might make the "
          "application quit...");

    return xlib_state->xlib_error_handler(display, e);
}

/**
 * @brief Map the X's KeyCodes to Scancode.
 *
 * Refering the glfw's implementation
 * https://github.com/glfw/glfw/blob/21fea01161e0d6b70c0c5c1f52dc8e7a7df14a50/src/x11_init.c
 */
void mapKeycodesToScancodes(void) {
    // TODO: Error handling
    sassert_msg(xlib_state, "Windowing system is not initialized?");

    XkbDescPtr desc = xlib_state->xkb_desc;

    u32 which = XkbKeyNamesMask | XkbKeyAliasesMask | XkbVirtualModNamesMask;

    XkbGetNames(xlib_state->display, which, desc);

    #if 0
    mapXKeyCodesToScancodes(
        (mapFunctionParams){
            .key_aliases = (XKeyAliasNameType *)desc->names->key_aliases,
            .key_names = (XKeyNameType *)desc->names->keys,
            .key_names_start_from_min_key_code = false,
            .max_key_code = desc->max_key_code,
            .min_key_code = desc->min_key_code,
            .num_key_aliases = desc->names->num_key_aliases},
        xlib_state->xKeyCode_to_Scancode);
    #endif

    XkbFreeNames(desc, which, false);
}

/**
 * @brief Implementation for xlib.
 *
 * Call with state NULL to get the size to be allocated and call once again
 * with pointer to the allocated memory to actually initialize.
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

    xlib_state = (XlibState *)state;

    // Set everything to 0
    sMemZeroOut(xlib_state, sizeof(XlibState));

    // Open Connection to X server
    xlib_state->display = XOpenDisplay(NULL);
    if (!xlib_state->display) {
        sError("Faild to open connection to X server via xlib");
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
    attribs.event_mask = ExposureMask | StructureNotifyMask;
    // attribs.event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask
    //                    | ButtonReleaseMask | PointerMotionMask |
    //                    ExposureMask | StructureNotifyMask;

    // Create the main window
    xlib_state->app_window = XCreateWindow(
        xlib_state->display, xlib_state->root_window, config->x, config->y,
        config->width, config->height, 0, CopyFromParent, InputOutput,
        CopyFromParent, attrib_mask, &attribs);

    // Query the availability of the extensions and their versions
    if (!XQueryExtension(xlib_state->display, "XInputExtension",
                         &xlib_state->xi_opcode, &xlib_state->xi_event_code,
                         &xlib_state->xi_error_code)) {
        sError("XInputExtension is not available");
        return false;
    }

    // i32 major = XI_2_Major, minor = XI_2_Minor;
    i32 major = 2, minor = 4;
    if (XIQueryVersion(xlib_state->display, &major, &minor)) {
        sError("XI2 max version supported by server is %d.%d.", major, minor);
        return false;
    }

    major = XkbMajorVersion, minor = XkbMinorVersion;
    if (!XkbQueryExtension(xlib_state->display, &xlib_state->xkb_opcode,
                           &xlib_state->xkb_event_code,
                           &xlib_state->xkb_error_code, &major, &minor)) {
        sError("Compatible version of XKB is not found in the server");
    }

    // xlib_state->xkb_desc = XkbGetKeyboard(xlib_state->display,
    //                                       XkbAllComponentsMask,
    //                                       XkbUseCoreKbd);
    xlib_state->xkb_desc =
        XkbGetMap(xlib_state->display, XkbAllMapComponentsMask, XkbUseCoreKbd);

    mapKeycodesToScancodes();

    // Select Xkb events
    // TODO: Register for xkb events and track the changes to the keyboard
    // to update the keycodes to scancode map
    u32 xkb_masks =
        XkbNewKeyboardNotifyMask | XkbMapNotifyMask | XkbNamesNotifyMask;
    XkbSelectEvents(xlib_state->display, XkbUseCoreKbd, xkb_masks, xkb_masks);

    // Select the XInput2 events

    // Similar to bit maks using byte array, where each bit represents a
    // mask and initialize to 0. Understood this concept from the chatgpt :)
    u8 masks[XIMaskLen(XI_LASTEVENT)] = {0};
    XIEventMask emask = {//.deviceid = XIAllDevices,
                         .deviceid = XIAllMasterDevices,
                         .mask = masks,
                         .mask_len = XIMaskLen(XI_LASTEVENT)};
    XISetMask(emask.mask, XI_ButtonPress);
    XISetMask(emask.mask, XI_ButtonRelease);
    XISetMask(emask.mask, XI_KeyPress);
    XISetMask(emask.mask, XI_KeyRelease);
    XISetMask(emask.mask, XI_Motion);
    if (XISelectEvents(xlib_state->display, xlib_state->app_window, &emask,
                       1)) {
        sError("Failed to select the input events using XInput2 extension.");
        return false;
    }

    // Atoms
    xlib_state->wm_delete_window =
        XInternAtom(xlib_state->display, "WM_DELETE_WINDOW", false);

    // Get notified when the window is getting destroyed
    XSetWMProtocols(xlib_state->display, xlib_state->app_window,
                    &xlib_state->wm_delete_window, 1);

    // Set class hint
    XClassHint class_hint = {.res_name = (c8 *)config->name,
                             .res_class = (c8 *)config->name};
    XSetClassHint(xlib_state->display, xlib_state->app_window, &class_hint);

    c8 *app_name = sStringConcatC8(config->name, " - X11(Xlib)", 0, 13, NULL);
    if (!platformSetWindowTitle(app_name))
        sError("Couldn't set the window title");
    sFree(app_name);

    // Todo: Make it as a parameter may be
    if (!platformSetWindowVisible(true)) sError("Couldn't show the window");

    // Make sure to flush so that everything will be sent to the server
    XFlush(xlib_state->display);

    // Sync the modifiers state
    syncKeymodsState(xlib_state->display);

    return true;
}

/**
 * @brief Implementation of xlib.
 */
void shutdownPlatformWindowing(void) {
    sassert_msg(xlib_state,
                "Shutting down windowing system twice or not initialized?");
    if (xlib_state->xkb_desc) XkbFreeKeyboard(xlib_state->xkb_desc, 0, true);
    if (xlib_state->display) {
        XDestroyWindow(xlib_state->display, xlib_state->app_window);
        XCloseDisplay(xlib_state->display);
    }
}

/**
 * @brief Helper function for handling the Generic events.
 */
void handleGenericEvents(XEvent *event) {
    if (event->xcookie.extension == xlib_state->xi_opcode) {
        if (!XGetEventData(xlib_state->display, &event->xcookie)) {
            sWarn("Failed to get the data from the cookie for XI");
            return;
        }
        XIDeviceEvent *device_event = (XIDeviceEvent *)event->xcookie.data;
        // XIRawEvent *raw_event = (XIRawEvent *)event.xcookie.data;
        switch (event->xcookie.evtype) {
            case XI_ButtonPress: {
                // sDebug("Button press: device=%d, button=%d",
                //        device_event->deviceid,
                //        device_event->detail);
                if (device_event->detail < 4) {
                    // Left = 1, right = 3, middle = 2
                    inputProcessButton(
                        device_event->detail, device_event->event_x,
                        device_event->event_y, getKeymods(), true);
                } else {
                    // TODO: Peek at the next event till the next
                    // TODO: event is not scroll and then pass delta
                    // TODO: as the number of scroll events in the
                    // TODO: same direction

                    // scroll:
                    //  up = 4 down = 5 left = 6 right = 7
                    inputProcessScroll((device_event->detail - 3), getKeymods(),
                                       1);
                }
            } break;
            case XI_ButtonRelease: {
                // sDebug("Button release: device:%d, button=%d",
                //        device_event->deviceid,
                //        device_event->detail);
                if (device_event->detail < 4) {
                    // Left = 1, right = 3, middle = 2
                    inputProcessButton(
                        device_event->detail, device_event->event_x,
                        device_event->event_y, getKeymods(), false);
                }
                // Scroll is being processed in button press event
            } break;

            case XI_KeyPress:
            case XI_KeyRelease: {
                KeySym keysym;
                u32 mods;
                if (!XkbTranslateKeyCode(
                        xlib_state->xkb_desc, device_event->detail,
                        device_event->mods.effective, &mods, &keysym))
                    sError("Failed to translate keycode");

                // c8 buf[64] = {0};
                // if (XLookupString(&ke, buf, sizeof(buf), &keysym, NULL))
                //     sDebug("XLookupString: '%s'", buf);
                sDebug("Keysym = %ld", keysym);
                // sDebug("Actual keycode = %lu, KeysymToKeycode = %lu",
                //        device_event->detail,
                //        getKeycodeFromKeySym(xlib_state->display,
                //        keysym));
                // Scancode sc =
                //     xlib_state->xKeyCode_to_Scancode[device_event->detail];
                Keycode kc = getKeycodeFromKeySym(keysym);
                b8 pressed = device_event->evtype == XI_KeyPress;
                updateKeymodsState(keysym, pressed);
                // Translating the X's keycode to linux's keycode by -8
                // Might not be reliable. Yet to figure out
                inputProcessKey(
                    getScancodeFromLinuxKeycode(device_event->detail - 8), kc,
                    getKeymods(), pressed,
                    (device_event->evtype == XI_KeyPress)
                        ? (device_event->flags & XIKeyRepeat)
                        : false);
            } break;
            case XI_Motion: {
                inputProcessPointerMotion(device_event->event_x,
                                          device_event->event_y);
            } break;

            default: {
                sError("Event type %d is not handled in XI's events?",
                       event->xcookie.evtype);
            } break;
        }
        // sDebug("valuators.data = (%f, %f)",
        //        device_event->valuators.values[0],
        //        device_event->valuators.values[1]);
        XFreeEventData(xlib_state->display, &event->xcookie);
    }
}

/**
 * @brief Loop through all the messages and fire the corresponding events.
 *
 * @return Returns false if application quit was recieved else true.
 */
b8 platformWindowPumpMessages(void) {
    sassert_msg(xlib_state, "Windowing system is not initialized?");
    b8 quit = false;

    XkbEvent xkb_event;
    XEvent *event = &xkb_event.core;

    // XNextEvent is blocking which means if direcly used then this function
    // will not return with true, i.e., utill the application recieves the
    // quit signal the loop will not stop. So use XPending ? Do I need to
    // check for !quit
    while (!quit && XPending(xlib_state->display)) {
        XNextEvent(xlib_state->display, event);
        if (xkb_event.type == xlib_state->xkb_event_code) {
            switch (xkb_event.any.xkb_type) {
                case XkbNewKeyboardNotify:
                    // sDebug("XkbNewKeyboardNotify");
                    // ? Should call this or not
                    mapKeycodesToScancodes();
                    break;
                case XkbMapNotify:
                    // sDebug("XkbMapNotify");
                    // ? Should call this or not
                    XkbGetUpdatedMap(xlib_state->display,
                                     XkbAllMapComponentsMask,
                                     xlib_state->xkb_desc);
                    mapKeycodesToScancodes();
                    break;
                case XkbNamesNotify:
                    // sDebug("XkbNamesNotify");
                    // ? Should call this or not
                    mapKeycodesToScancodes();
                default:
                    sError("Event type %d from xkb is not being handled?",
                           xkb_event.any.xkb_type);
            }
            continue;
        }

        switch (event->type) {
            case Expose:
                // TODO:
                break;
            case ConfigureNotify:
                // TODO:
                break;
            case ClientMessage: {
                if ((unsigned long)event->xclient.data.l[0]
                    == xlib_state->wm_delete_window) {
                    fireEvent(EVENT_CODE_APPLICATION_QUIT, NULL,
                              ((EventContext){0}));
                    quit = true;
                }
            } break;
            case GenericEvent: {
                handleGenericEvents(event);
            } break;
            case MappingNotify:
                // Ignore this event since we are handling it in the xkb
                // events
                break;
            default: {
                sTrace("An event is being ignored: Event type: %d",
                       event->type);
            } break;
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
 * If called with true even if the window is visible, or called with false
 * even if the window is not visible, no error will be generated.
 *
 * @param visibility if true make window visible
 *
 * @return Returns true if changes were made successfully.
 */
b8 platformSetWindowVisible(b8 visible) {
    sassert_msg(xlib_state, "Windowing system is not initialized?");
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
b8 platformSetWindowTitle(const c8 *title) {
    sassert_msg(xlib_state, "Windowing system is not initialized?");
    XStoreName(xlib_state->display, xlib_state->app_window, title);
    return true;
}

/**
 * @brief Get the title of the window (xlib implementation).
 *
 * @return Returns the malloced stirng, user should call sFree.
 */
c8 *platformGetWindowTitle(void) {
    sassert_msg(xlib_state, "Windowing system is not initialized?");
    c8 *title;
    if (XFetchName(xlib_state->display, xlib_state->app_window, &title)) {
        c8 *ret = sStringCopyC8(title, 0);
        XFree(title);
        return ret;
    }

    return NULL;
}

#endif
