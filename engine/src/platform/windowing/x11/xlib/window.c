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

    #include "core/assertions.h"
    #include "core/event.h"
    #include "core/logger.h"
    #include "core/memory.h"
    #include "core/sstring.h"
    #include "input/input.h"

typedef struct XlibState {
        Display *display;
        i32 screen, screen_width, screen_height;
        i32 xi_opcode, xi_event_code, xi_error_code;
        i32 xkb_opcode, xkb_event_code, xkb_error_code;
        Window root_window, app_window;
        Scancode xkeyCode_to_scancode[256];
        u64 white_pixel, black_pixel;
        i32 (*xlib_error_handler)(Display *, XErrorEvent *);
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
i32 handleXErrors(Display *display, XErrorEvent *e) {
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
 * @brief Map the X's KeyCodes to Scancode.
 *
 * Refering the glfw's implementation
 * https://github.com/glfw/glfw/blob/21fea01161e0d6b70c0c5c1f52dc8e7a7df14a50/src/x11_init.c
 */
void mapKeycodesToScancodes() {
    // TODO: Error handling
    sassert_msg(xlib_state, "Windowing system is not initialized?");

    XkbDescPtr desc = XkbGetMap(xlib_state->display, 0, XkbUseCoreKbd);
    XkbGetNames(xlib_state->display, XkbKeyNamesMask | XkbKeyAliasesMask, desc);

    const struct {
            char *name;
            Scancode code;
    } name_code_map[] = {
        // Look at the xev output as well as the
        // /usr/share/X11/xkb/keycodes/evdev
        {"TLDE",            SCANCODE_GRAVE},
        {"AE01",                SCANCODE_1},
        {"AE02",                SCANCODE_2},
        {"AE03",                SCANCODE_3},
        {"AE04",                SCANCODE_4},
        {"AE05",                SCANCODE_5},
        {"AE06",                SCANCODE_6},
        {"AE07",                SCANCODE_7},
        {"AE08",                SCANCODE_8},
        {"AE09",                SCANCODE_9},
        {"AE10",                SCANCODE_0},
        {"AE11",            SCANCODE_MINUS},
        {"AE12",           SCANCODE_EQUALS},
        {"BKSP",        SCANCODE_BACKSPACE},

        { "TAB",              SCANCODE_TAB},
        {"AD01",                SCANCODE_Q},
        {"AD02",                SCANCODE_W},
        {"AD03",                SCANCODE_E},
        {"AD04",                SCANCODE_R},
        {"AD05",                SCANCODE_T},
        {"AD06",                SCANCODE_Y},
        {"AD07",                SCANCODE_U},
        {"AD08",                SCANCODE_I},
        {"AD09",                SCANCODE_O},
        {"AD10",                SCANCODE_P},
        {"AD11",     SCANCODE_LEFT_BRACKET},
        {"AD12",    SCANCODE_RIGHT_BRACKET},
        {"BKSL",        SCANCODE_BACKSLASH},
        {"RTRN",            SCANCODE_ENTER},

        {"CAPS",        SCANCODE_CAPS_LOCK},
        {"AC01",                SCANCODE_A},
        {"AC02",                SCANCODE_S},
        {"AC03",                SCANCODE_D},
        {"AC04",                SCANCODE_F},
        {"AC05",                SCANCODE_G},
        {"AC06",                SCANCODE_H},
        {"AC07",                SCANCODE_J},
        {"AC08",                SCANCODE_K},
        {"AC09",                SCANCODE_L},
        {"AC10",        SCANCODE_SEMICOLON},
        {"AC11",       SCANCODE_APOSTROPHE},
        {"AC12",        SCANCODE_BACKSLASH},

        {"LFSH",       SCANCODE_LEFT_SHIFT},
        {"LSGT", SCANCODE_NON_US_BACKSLASH},
        {"AB01",                SCANCODE_Z},
        {"AB02",                SCANCODE_X},
        {"AB03",                SCANCODE_C},
        {"AB04",                SCANCODE_V},
        {"AB05",                SCANCODE_B},
        {"AB06",                SCANCODE_N},
        {"AB07",                SCANCODE_M},
        {"AB08",            SCANCODE_COMMA},
        {"AB09",           SCANCODE_PERIOD},
        {"AB10",            SCANCODE_SLASH},
        {"RTSH",      SCANCODE_RIGHT_SHIFT},

        {"LCTL",     SCANCODE_LEFT_CONTROL},
        {"LWIN",         SCANCODE_LEFT_GUI},
        {"LALT",         SCANCODE_LEFT_ALT},
        {"SPCE",         SCANCODE_SPACEBAR},
        {"RALT",        SCANCODE_RIGHT_ALT},
        {"ALGR",        SCANCODE_RIGHT_ALT},
        {"RWIN",        SCANCODE_RIGHT_GUI},
        {"COMP",             SCANCODE_MENU},
        {"MENU",             SCANCODE_MENU},
        {"RCTL",    SCANCODE_RIGHT_CONTROL},

        { "ESC",           SCANCODE_ESCAPE},
        {"FK01",               SCANCODE_F1},
        {"FK02",               SCANCODE_F2},
        {"FK03",               SCANCODE_F3},
        {"FK04",               SCANCODE_F4},
        {"FK05",               SCANCODE_F5},
        {"FK06",               SCANCODE_F6},
        {"FK07",               SCANCODE_F7},
        {"FK08",               SCANCODE_F8},
        {"FK09",               SCANCODE_F9},
        {"FK10",              SCANCODE_F10},
        {"FK11",              SCANCODE_F11},
        {"FK12",              SCANCODE_F12},

        {"PRSC",     SCANCODE_PRINT_SCREEN},
        {"SCLK",      SCANCODE_SCROLL_LOCK},
        {"PAUS",            SCANCODE_PAUSE},

        { "INS",           SCANCODE_INSERT},
        {"HOME",             SCANCODE_HOME},
        {"PGUP",          SCANCODE_PAGE_UP},
        {"DELE",           SCANCODE_DELETE},
        { "END",              SCANCODE_END},
        {"PGDN",        SCANCODE_PAGE_DOWN},

        {  "UP",         SCANCODE_UP_ARROW},
        {"LEFT",       SCANCODE_LEFT_ARROW},
        {"DOWN",       SCANCODE_DOWN_ARROW},
        {"RGHT",      SCANCODE_RIGHT_ARROW},

        {"NMLK",         SCANCODE_NUM_LOCK},
        {"KPDV",    SCANCODE_KEYPAD_DIVIDE},
        {"KPMU",  SCANCODE_KEYPAD_MULTIPLY},
        {"KPSU",     SCANCODE_KEYPAD_MINUS},

        { "KP7",         SCANCODE_KEYPAD_7},
        { "KP8",         SCANCODE_KEYPAD_8},
        { "KP9",         SCANCODE_KEYPAD_9},
        {"KPAD",      SCANCODE_KEYPAD_PLUS},

        { "KP4",         SCANCODE_KEYPAD_4},
        { "KP5",         SCANCODE_KEYPAD_5},
        { "KP6",         SCANCODE_KEYPAD_6},

        { "KP1",         SCANCODE_KEYPAD_1},
        { "KP2",         SCANCODE_KEYPAD_2},
        { "KP3",         SCANCODE_KEYPAD_3},
        {"KPEN",     SCANCODE_KEYPAD_ENTER},

        { "KP0",         SCANCODE_KEYPAD_0},
        {"KPDL",    SCANCODE_KEYPAD_PERIOD},
        {"KPEQ",    SCANCODE_KEYPAD_EQUALS},

        {"FK13",              SCANCODE_F13},
        {"FK14",              SCANCODE_F14},
        {"FK15",              SCANCODE_F15},
        {"FK16",              SCANCODE_F16},
        {"FK17",              SCANCODE_F17},
        {"FK18",              SCANCODE_F18},
        {"FK19",              SCANCODE_F19},
        {"FK20",              SCANCODE_F20},
        {"FK21",              SCANCODE_F21},
        {"FK22",              SCANCODE_F22},
        {"FK23",              SCANCODE_F23},
        {"FK24",              SCANCODE_F24},
        // TODO: Add the other codes
        // NOTE: Have already added most of the commonly used ones
    };

    u32 name_code_map_len = sizeof(name_code_map) / sizeof(name_code_map[0]);

    for (u32 keycode = desc->min_key_code; keycode <= desc->max_key_code;
         ++keycode) {
        for (u32 i = 0; i < name_code_map_len; ++i) {
            if (sStringEqual(desc->names->keys[keycode].name,
                             name_code_map[i].name, XkbKeyNameLength)) {
                xlib_state->xkeyCode_to_scancode[keycode] =
                    name_code_map[i].code;
                break;
            }
        }

        if (xlib_state->xkeyCode_to_scancode[keycode] != SCANCODE_NONE)
            continue;

        // Look at the aliases if not found
        for (u32 i = 0; i < desc->names->num_key_aliases; ++i) {
            if (xlib_state->xkeyCode_to_scancode[keycode] != SCANCODE_NONE)
                break;

            if (!sStringEqual(desc->names->key_aliases[i].real,
                              desc->names->keys[keycode].name,
                              XkbKeyNameLength))
                continue;

            for (u32 j = 0; j < name_code_map_len; ++j)
                if (sStringEqual(desc->names->key_aliases[i].alias,
                                 name_code_map[j].name, XkbKeyNameLength))
                    xlib_state->xkeyCode_to_scancode[keycode] =
                        name_code_map[j].code;
        }
    }

    XkbFreeNames(desc, XkbKeyNamesMask | XkbKeyAliasesMask, true);
    XkbFreeKeyboard(desc, 0, true);
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
    //                    | ButtonReleaseMask | PointerMotionMask | ExposureMask
    //                    | StructureNotifyMask;

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

    mapKeycodesToScancodes();

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
    XClassHint class_hint = {.res_name = (char *)config->name,
                             .res_class = (char *)config->name};
    XSetClassHint(xlib_state->display, xlib_state->app_window, &class_hint);

    char *app_name = sStringConcat(config->name, " - X11(Xlib)", 0, 13, NULL);
    if (!platformSetWindowTitle(app_name))
        sError("Couldn't set the window title");
    sFree(app_name);

    // Todo: Make it as a parameter may be
    if (!platformSetWindowVisible(true)) sError("Couldn't show the window");

    // Make sure to flush so that everything will be sent to the server
    XFlush(xlib_state->display);

    return true;
}

/**
 * @brief Implementation of xlib.
 *
 * @param state Pointer to the allocated memory
 */
void shutdownPlatformWindowing(void *state) {
    sassert_msg(xlib_state,
                "Shutting down windowing system twice or not initialized?");
    UNUSED(state);
    if (xlib_state->display) {
        XDestroyWindow(xlib_state->display, xlib_state->app_window);
        XCloseDisplay(xlib_state->display);
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

    XEvent event;

    // XPending is equivalent to XEventsQueued(display, QueuedAfterFlush)
    // XNextEvent is blocking which means if direcly used then this function
    // will not return with true, i.e., utill the application recieves the quit
    // signal the loop will not stop. So use XPending
    // ? Do I need to check for !quit
    while (!quit && XPending(xlib_state->display)) {
        XNextEvent(xlib_state->display, &event);
        switch (event.type) {
            case Expose:
                // TODO:
                break;
            case ConfigureNotify:
                // TODO:
                break;
            case ClientMessage: {
                if ((unsigned long)event.xclient.data.l[0]
                    == xlib_state->wm_delete_window) {
                    fireEvent(EVENT_CODE_APPLICATION_QUIT, NULL,
                              ((EventContext){0}));
                    quit = true;
                }
            } break;
            case GenericEvent: {
                if (event.xcookie.extension == xlib_state->xi_opcode) {
                    if (!XGetEventData(xlib_state->display, &event.xcookie)) {
                        sWarn("Failed to get the data from the cookie for XI");
                        break;
                    }
                    XIDeviceEvent *device_event =
                        (XIDeviceEvent *)event.xcookie.data;
                    // XIRawEvent *raw_event = (XIRawEvent *)event.xcookie.data;
                    switch (event.xcookie.evtype) {
                        case XI_ButtonPress: {
                            // sDebug("Button press: device=%d, button=%d",
                            //        device_event->deviceid,
                            //        device_event->detail);
                            if (device_event->detail < 4) {
                                // Left = 1, right = 3, middle = 2
                                inputProcessButton(device_event->detail, true);
                            } else {
                                // TODO: Peek at the next event till the next
                                // TODO: event is not scroll and then pass delta
                                // TODO: as the number of scroll events in the
                                // TODO: same direction

                                // scroll:
                                //  up = 4 down = 5 left = 6 right = 7
                                inputProcessScroll((device_event->detail - 3),
                                                   1);
                            }
                        } break;
                        case XI_ButtonRelease: {
                            // sDebug("Button release: device:%d, button=%d",
                            //        device_event->deviceid,
                            //        device_event->detail);
                            if (device_event->detail < 4) {
                                // Left = 1, right = 3, middle = 2
                                inputProcessButton(device_event->detail, false);
                            }
                            // Scroll is being processed in button press event
                        } break;
                        case XI_KeyPress: {
                            // sDebug("Key press: device:%d, keycode=%d%s",
                            //        device_event->deviceid,
                            //        device_event->detail, (device_event->flags
                            //        & XIKeyRepeat)
                            //            ? " KeyRepeat"
                            //            : "");
                            inputProcessKey(
                                xlib_state->xkeyCode_to_scancode[device_event
                                                                     ->detail],
                                true, (device_event->flags & XIKeyRepeat));
                        } break;
                        case XI_KeyRelease: {
                            // sDebug("Key release: device:%d, keycode=%d",
                            //        device_event->deviceid,
                            //        device_event->detail);
                            inputProcessKey(
                                xlib_state->xkeyCode_to_scancode[device_event
                                                                     ->detail],
                                false, false);
                        } break;
                        case XI_Motion: {
                            inputProcessPointerMotion(device_event->event_x,
                                                      device_event->event_y);
                        } break;

                        default: {
                            sError("Should not be getting some unkown event "
                                   "from the XI.");
                        } break;
                    }
                    // sDebug("valuators.data = (%f, %f)",
                    //        device_event->valuators.values[0],
                    //        device_event->valuators.values[1]);
                    XFreeEventData(xlib_state->display, &event.xcookie);
                }
            } break;
            default: {
                sTrace("An event is being ignored: Event type: %d", event.type);
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
b8 platformSetWindowTitle(const char *title) {
    sassert_msg(xlib_state, "Windowing system is not initialized?");
    XStoreName(xlib_state->display, xlib_state->app_window, title);
    return true;
}

/**
 * @brief Get the title of the window (xlib implementation).
 *
 * @return Returns the malloced stirng, user should call sFree.
 */
char *platformGetWindowTitle(void) {
    sassert_msg(xlib_state, "Windowing system is not initialized?");
    char *title;
    if (XFetchName(xlib_state->display, xlib_state->app_window, &title)) {
        char *ret = sStringCopy(title, 0);
        XFree(title);
        return ret;
    }

    return NULL;
}

#endif
