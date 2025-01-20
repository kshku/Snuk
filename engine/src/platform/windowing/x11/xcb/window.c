#include "../../../window.h"

#ifdef SPLATFORM_WINDOWING_X11_XCB
// NOTE: Just go to /usr/include/xcb and read header files to know what are
// NOTE: things we can use. Really there are no good docs :( Also for the
// NOTE: header files we include there might be separate libraries which
// NOTE: need to be used. Go to /usr/lib and run 'ls | grep xcb'

// Documentation used
// https://www.x.org/releases/current/doc/libX11/libX11/libX11.html
// https://www.x.org/releases/X11R7.7/doc/libxcb/tutorial/index.html
// https://xcb.freedesktop.org/manual/index.html

// Basic Graphics Programming with the Xlib library
// https://ftp.dim13.org/pub/doc/Xlib.pdf

    #include <stdlib.h>
    #include <xcb/xcb.h>
    #include <xcb/xcb_icccm.h>
    #include <xcb/xinput.h>
    #include <xcb/xkb.h>
    #include <xkbcommon/xkbcommon-x11.h>

    #include "../../linux_input_helper.h"
    #include "core/assertions.h"
    #include "core/event.h"
    #include "core/logger.h"
    #include "core/memory/memory.h"
    #include "core/sstring.h"
    #include "input/input.h"

typedef struct XCBState {
        // Display *display;
        xcb_connection_t *connection;
        xcb_screen_t *screen;
        xcb_window_t app_window;
        xcb_atom_t wm_delete_window, wm_protocols;
        u8 xi_opcode, xi_event_code, xi_error_code;
        u8 xkb_opcode, xkb_event_code, xkb_error_code;

        Scancode xKeyCodeToScancode[256];
        struct xkb_context *xkb_context;
        struct xkb_keymap *xkb_keymap;
        struct xkb_state *xkb_state;
} XCBState;

static XCBState *xcb_state;

/**
 * @brief Map the X's KeyCodes to Scancode.
 */
void mapKeycodesToScancodes(void) {
    u32 which = XCB_XKB_NAME_DETAIL_KEY_NAMES | XCB_XKB_NAME_DETAIL_KEY_ALIASES;
    xcb_xkb_get_names_cookie_t names_cookie = xcb_xkb_get_names(
        xcb_state->connection, XCB_XKB_ID_USE_CORE_KBD, which);
    xcb_xkb_get_names_reply_t *names_reply =
        xcb_xkb_get_names_reply(xcb_state->connection, names_cookie, NULL);

    // Don't ask me how I did it. Did a lot of trail and errors using the
    // functions found in the header file. Here is the comments I wrote while I
    // was doing so.

    // void *value_list = xcb_xkb_get_names_value_list(names_reply);
    // void *value_list_2 = xcb_xkb_get_names_value_list(names_reply);
    // i32 ret;
    // I was getting error "Cannot access the memory" or something.
    // Searched for all the functions starting with xcb_xkb_get_names and
    // found unpack and serialize functions. They return int don't know what
    // return value is used for(Probably true or false of course). By the
    // meaning guessing it should be called on the value list used it and
    // seems like it works!

    // Just passed parameters by guessing and didn't know what to pass for
    // buffer and aux so passed value_list if it works then no problem.

    // Even after calling serialize the names were not in order. After
    // inspecting the memory content the return value of
    // xcb_get_names_value_list function is not the
    // xcb_get_names_value_list_t but really arbitary data (which is
    // requested through which paramter). but don't know what is aux is
    // used. But if it is NULL it will be error so creating another
    // value_list and passing that as buffer might work
    // ret = xcb_xkb_get_names_value_list_unpack(
    //     value_list_2, names_reply->nTypes, names_reply->indicators,
    //     names_reply->virtualMods, names_reply->groupNames,
    //     names_reply->nKeys, names_reply->nKeyAliases,
    //     names_reply->nRadioGroups, XCB_XKB_NAME_DETAIL_KEY_NAMES |
    //     XCB_XKB_NAME_DETAIL_KEY_ALIASES, value_list);
    // sDebug("ret = %d", ret);

    // Looks like unpacking is not enough. I need to serialize it too.
    // In Xlib we can access the names like desc->names->keys[keycode].name and
    // it will give the names for respective keycodes. But in xcb whithout
    // serialize function it looked like out of order. Gussing and passing
    // parameters.

    // ret = xcb_xkb_get_names_value_list_serialize(
    //     &value_list, names_reply->nTypes, names_reply->indicators,
    //     names_reply->virtualMods, names_reply->groupNames,
    //     names_reply->nKeys, names_reply->nKeyAliases,
    //     names_reply->nRadioGroups, XCB_XKB_NAME_DETAIL_KEY_NAMES |
    //     XCB_XKB_NAME_DETAIL_KEY_ALIASES, value_list_2);
    // sDebug("ret = %d", ret);
    // Well after asking the questions to chatGPT and getting absolutely wrong
    // answers, from it's explanations I got an idea. I thought in XCB some of
    // the things are just pointer manipulation beyond the declared data types
    // and all(Look at the  we select the events in XI or X input extension).
    // But it is just that everywhere

    xcb_xkb_get_names_value_list_t value_list;
    // Inspected the memory, It is also just pointer manipulation. Points right
    // after the sizeof of the xcb_xkb_get_names_reply_t structure in the
    // names_reply
    void *buf = xcb_xkb_get_names_value_list(names_reply);
    // So we pass this buffer to the unpack like this? Shold we use the
    // serialize or not(If things will work then no need to call serialize may
    // be).
    xcb_xkb_get_names_value_list_unpack(
        buf, names_reply->nTypes, names_reply->indicators,
        names_reply->virtualMods, names_reply->groupNames, names_reply->nKeys,
        names_reply->nKeyAliases, names_reply->nRadioGroups, which,
        &value_list);
    // And... Yeah, the order is still not correct may be should call the
    // serialize, but why pointer to buf?
    // xcb_xkb_get_names_value_list_serialize(
    //     &buf, names_reply->nTypes, names_reply->indicators,
    //     names_reply->virtualMods, names_reply->groupNames,
    //     names_reply->nKeys, names_reply->nKeyAliases,
    //     names_reply->nRadioGroups, which, &value_list);
    // Still didn't work... Oh got it. Taking pointer to the buf since it
    // changes the order. So first call serialize then unpack, but why serialize
    // takes value_list? Is serialize alone is enough? No it value list as
    // constant which means it is not going to manipulate. Also if we call
    // serialize first, it throws error.

    // Finally got it. In Xlib we could get the name of keycode directly as said
    // above in comment somewhere, since the array starts at exactly from
    // keycode 0, even though it is not the minimum keycode, in xcb it looks
    // like the array[0] is the name of the keycode minimum(Inspect the memory).
    // Also I don't think we need to call the serialize function.  // Added one
    // more parameter to the mapXKeyCodesToScancodes function. It takes a lot of
    // parameters, so time for refactor it by creating a new struct and take it
    // as the parameter

    // Haha, finally realized that I am just mapping here and didn't use it
    // while passing it to process the input, all those wrong mappings are due
    // to it. But still that parameter to check whether the key names start from
    // min keycode or from zero is required. Thanks god, went in right way. But
    // still don't know should call serialize or not. So not calling it.

    // Can't we just access it directly like value_list->keyNames?
    // I don't know, they have those functions so I am just using it
    // I am here just guessing things and doing it. I couldn't find
    // documentation about it.
    // mapFunctionParams params = {
    //     .key_aliases =
    //         (XKeyAliasNameType *)xcb_xkb_get_names_value_list_key_aliases(
    //             &value_list),
    //     .key_names =
    //         (XKeyNameType
    //         *)xcb_xkb_get_names_value_list_key_names(&value_list),
    //     .key_names_start_from_min_key_code = true,
    //     .max_key_code = names_reply->maxKeyCode,
    //     .min_key_code = names_reply->minKeyCode,
    //     .num_key_aliases = names_reply->nKeyAliases};
    // mapXKeyCodesToScancodes(params, xcb_state->xKeyCodeToScancode);

    // If I try to free value list, it gives "double free or corruption
    // (!prev)" error. So don't free(value_list)

    free(names_reply);
}

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

    sMemZeroOut(xcb_state, sizeof(XCBState));

    // TODO: Error handling
    // Open connection to X server
    // if (!(xcb_state->display = XOpenDisplay(NULL))) {
    //     sError("Faild to open connection to X server");
    //     return false;
    // }

    // xcb_state->connection = XGetXCBConnection(xcb_state->display);

    i32 screen_number;
    xcb_state->connection = xcb_connect(NULL, &screen_number);
    if (xcb_connection_has_error(xcb_state->connection)) {
        sError("Faild to connect X server via xcb");
        return false;
    }

    // screen_number = DefaultScreen(xcb_state->display);

    // Getting screen number
    xcb_screen_iterator_t iter =
        xcb_setup_roots_iterator(xcb_get_setup(xcb_state->connection));
    for (i32 i = 0; i < screen_number; ++i) xcb_screen_next(&iter);
    xcb_state->screen = iter.data;

    // Generate id for window
    xcb_state->app_window = xcb_generate_id(xcb_state->connection);

    // Similar to XSetWindowAttributes
    u32 value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    u32 event_mask =
        XCB_EVENT_MASK_EXPOSURE
        | XCB_EVENT_MASK_STRUCTURE_NOTIFY;  // u32 event_mask =
                                            // XCB_EVENT_MASK_KEY_PRESS |
                                            // XCB_EVENT_MASK_KEY_RELEASE
    //                | XCB_EVENT_MASK_BUTTON_PRESS |
    //                XCB_EVENT_MASK_BUTTON_RELEASE |
    //                XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_EXPOSURE |
    //                XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    u32 value_list[2] = {xcb_state->screen->black_pixel, event_mask};

    // Create window
    xcb_create_window(xcb_state->connection, XCB_COPY_FROM_PARENT,
                      xcb_state->app_window, xcb_state->screen->root, config->x,
                      config->y, config->width, config->height, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      xcb_state->screen->root_visual, value_mask, value_list);

    // Query the availability of the extensions and their versions

    // Send all the extension queries and get the reply below
    xcb_query_extension_cookie_t xi_cookie =
        xcb_query_extension(xcb_state->connection, 15, "XInputExtension");
    xcb_xkb_use_extension_cookie_t xkb_cookie = xcb_xkb_use_extension(
        xcb_state->connection, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);

    xcb_query_extension_reply_t *xi_reply =
        xcb_query_extension_reply(xcb_state->connection, xi_cookie, NULL);

    if (!xi_reply || !xi_reply->present) {
        sError("XInputExtension is not available");
        free(xi_reply);
        return false;
    }

    xcb_state->xi_opcode = xi_reply->major_opcode;
    xcb_state->xi_event_code = xi_reply->first_event;
    xcb_state->xi_error_code = xi_reply->first_error;

    free(xi_reply);

    // Get the xkb data
    const xcb_query_extension_reply_t *xkb_data =
        xcb_get_extension_data(xcb_state->connection, &xcb_xkb_id);
    if (!xkb_data || !xkb_data->present) {
        sError("Xkb extension is not found");
        return false;
    }
    xcb_state->xkb_opcode = xkb_data->major_opcode;
    xcb_state->xkb_error_code = xkb_data->first_error;
    xcb_state->xkb_event_code = xkb_data->first_event;

    xcb_xkb_use_extension_reply_t *xkb_reply =
        xcb_xkb_use_extension_reply(xcb_state->connection, xkb_cookie, NULL);
    if (!xkb_reply || !xkb_reply->supported) {
        sError("Failed to use the XKB extension");
        free(xkb_reply);
        return false;
    }

    free(xkb_reply);

    mapKeycodesToScancodes();

    // Select Xkb events
    // TODO: Register for xkb events and track the changes to the keyboard to
    // update the keycodes to scancode map
    u16 xkb_masks =
        XCB_XKB_EVENT_TYPE_NEW_KEYBOARD_NOTIFY | XCB_XKB_EVENT_TYPE_MAP_NOTIFY
        | XCB_XKB_EVENT_TYPE_STATE_NOTIFY | XCB_XKB_EVENT_TYPE_NAMES_NOTIFY;
    // Don't know about that map related params
    // https://github.com/i3/i3/blob/853b0d9161a5f99char491751590229a449e4076b6/src/main.c#L840
    // If affectMap and map are not set to 0xff, XCB_XKB_MAP_NOTIFY is not
    // triggered
    // How do they know how to use xcb?
    xcb_xkb_select_events(xcb_state->connection, XCB_XKB_ID_USE_CORE_KBD,
                          xkb_masks, 0, xkb_masks, 0xff, 0xff, NULL);

    // Setup the xkbcommon
    xcb_state->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!xcb_state->xkb_context) {
        sError("Failed to get the xkb context(xkbcommon)");
        return false;
    }
    i32 xkb_device_id =
        xkb_x11_get_core_keyboard_device_id(xcb_state->connection);
    if (xkb_device_id == -1) {
        sError("Failed to get the core keyboard device id(xkbcommon)");
        return false;
    }
    xcb_state->xkb_keymap = xkb_x11_keymap_new_from_device(
        xcb_state->xkb_context, xcb_state->connection, xkb_device_id,
        XKB_MAP_COMPILE_NO_FLAGS);
    if (!xcb_state->xkb_keymap) {
        sError("Failed to get the key map (xkbcommon)");
        return false;
    }
    xcb_state->xkb_state = xkb_x11_state_new_from_device(
        xcb_state->xkb_keymap, xcb_state->connection, xkb_device_id);
    if (!xcb_state->xkb_state) {
        sError("Failed to get the xkb state(xkbcommon)");
        return false;
    }

    i32 major = 2, minor = 4;
    // Can't put these above because these are not available if extension is not
    // there right?
    xcb_input_xi_query_version_cookie_t xi_ver_cookie =
        xcb_input_xi_query_version(xcb_state->connection, major, minor);

    xcb_input_xi_query_version_reply_t *xi_ver_reply =
        xcb_input_xi_query_version_reply(xcb_state->connection, xi_ver_cookie,
                                         NULL);

    if (xi_ver_reply->major_version != major
        || xi_ver_reply->minor_version != minor) {
        sError("XI2 max version supported by server is %d.%d.",
               xi_ver_reply->major_version, xi_ver_reply->minor_version);
        free(xi_ver_reply);
        return false;
    }

    free(xi_ver_reply);

    // Select the XInput2 events
    // https://stackoverflow.com/questions/39641675/how-to-register-events-using-libxcb-xinput

    struct {
            xcb_input_event_mask_t header;
            xcb_input_xi_event_mask_t masks;
    } emask = {
        .header = {.deviceid = XCB_INPUT_DEVICE_ALL_MASTER,
                   .mask_len =
                       sizeof(emask.masks) / sizeof(u32) /* Basically 1 */},
        .masks = XCB_INPUT_XI_EVENT_MASK_BUTTON_PRESS
               | XCB_INPUT_XI_EVENT_MASK_BUTTON_RELEASE
               | XCB_INPUT_XI_EVENT_MASK_KEY_PRESS
               | XCB_INPUT_XI_EVENT_MASK_KEY_RELEASE
               | XCB_INPUT_XI_EVENT_MASK_MOTION
    };

    xcb_input_xi_select_events(xcb_state->connection, xcb_state->app_window, 1,
                               &emask.header);

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
    xcb_atom_t wm_protocol_atoms[1] = {xcb_state->wm_delete_window};
    xcb_icccm_set_wm_protocols(xcb_state->connection, xcb_state->app_window,
                               xcb_state->wm_protocols, 1, wm_protocol_atoms);

    // Set class hint
    u64 name_length = sStringLength(config->name);
    char *buf = sStringConcat(
        config->name, config->name,
        name_length + 1,  // name_length + 1 to copy the NULL character
        name_length, NULL);
    // sDebug("class = '%s' '%s'", buf, buf + name_length + 1);
    xcb_icccm_set_wm_class(xcb_state->connection, xcb_state->app_window,
                           (name_length * 2 + 1), buf);
    sFree(buf);

    char *app_name =
        sStringConcat(config->name, " - X11(XCB)", name_length, 12, NULL);
    if (!platformSetWindowTitle(app_name))
        sError("Couldn't set the window title");
    sFree(app_name);

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
 * @brief Implementation of xcb.
 */
void shutdownPlatformWindowing(void) {
    sassert_msg(xcb_state,
                "Shutting down windowing system twice or not initialized?");

    if (xcb_state->xkb_state) xkb_state_unref(xcb_state->xkb_state);
    if (xcb_state->xkb_keymap) xkb_keymap_unref(xcb_state->xkb_keymap);
    if (xcb_state->xkb_context) xkb_context_unref(xcb_state->xkb_context);

    xcb_destroy_window(xcb_state->connection, xcb_state->app_window);

    // if (xcb_state->display) XCloseDisplay(xcb_state->display);
    xcb_disconnect(xcb_state->connection);
}

/**
 * @brief Helper function for handling the Generic events.
 */
void handleGenericEvents(xcb_ge_generic_event_t *ge) {
    if (ge->extension == xcb_state->xi_opcode) {
        switch (ge->event_type) {
            case XCB_INPUT_BUTTON_PRESS: {
                xcb_input_button_release_event_t *bpe =
                    (xcb_input_button_release_event_t *)ge;
                // sDebug("Button press: device=%d, button =
                // %d",
                //        bpe->deviceid, bpe->detail);
                if (bpe->detail < 4) {
                    // Left = 1, right = 3, middle = 2
                    inputProcessButton(
                        bpe->detail, (bpe->event_x / (double)MAX_U16),
                        (bpe->event_y / (double)MAX_U16),
                        getKeymodsFromXKBCommon(xcb_state->xkb_keymap,
                                                xcb_state->xkb_state),
                        true);
                } else {
                    // ? Should we do it?
                    // TODO: Peek at the next event till the next event is not
                    // scroll and then pass delta as the number of scroll events
                    // in the same direction

                    // scroll:
                    // up = 4 down = 5 left = 6 right = 7
                    inputProcessScroll(
                        (bpe->detail - 3),
                        getKeymodsFromXKBCommon(xcb_state->xkb_keymap,
                                                xcb_state->xkb_state),
                        1);
                }
            } break;
            case XCB_INPUT_BUTTON_RELEASE: {
                xcb_input_button_press_event_t *bre =
                    (xcb_input_button_press_event_t *)ge;
                // sDebug("Button release: device:%d, button =
                // %d",
                //        bre->deviceid, bre->detail);
                if (bre->detail < 4) {
                    inputProcessButton(
                        bre->detail, (bre->event_x / (double)MAX_U16),
                        (bre->event_y / (double)MAX_U16),
                        getKeymodsFromXKBCommon(xcb_state->xkb_keymap,
                                                xcb_state->xkb_state),
                        false);
                }
            } break;
            case XCB_INPUT_KEY_PRESS: {
                xcb_input_key_press_event_t *kpe =
                    (xcb_input_key_press_event_t *)ge;

                enum xkb_state_component changed = xkb_state_update_key(
                    xcb_state->xkb_state, kpe->detail, XKB_KEY_DOWN);
                UNUSED(changed);

                xkb_keysym_t keysym = xkb_state_key_get_one_sym(
                    xcb_state->xkb_state, kpe->detail);

                // In quick guide they used 64 bytes
                char buf[64];
                xkb_keysym_get_name(keysym, buf, sizeof(buf));
                sDebug("Keysym to string result = '%s'", buf);

                // We can check whether the key should repeat as
                // if (kpe->flags & XCB_INPUT_KEY_EVENT_FLAGS_KEY_REPEAT
                //     && xkb_keymap_key_repeats(xcb_state->xkb_keymap,
                //                               kpe->detail))
                // We can use our helper getKeycodeFromKeySym function since
                // the value of keysym is same Scancode sc =
                // xcb_state->xKeyCodeToScancode[kpe->detail];
                Keycode kc = getKeycodeFromKeySym(keysym);
                // Translating the X's keycode to linux's keycode by -8
                // Might not be reliable. Yet to figure out
                inputProcessKey(
                    getScancodeFromLinuxKeycode(kpe->detail - 8), kc,
                    getKeymodsFromXKBCommon(xcb_state->xkb_keymap,
                                            xcb_state->xkb_state),
                    true, (kpe->flags & XCB_INPUT_KEY_EVENT_FLAGS_KEY_REPEAT));
            } break;
            case XCB_INPUT_KEY_RELEASE: {
                xcb_input_key_release_event_t *kre =
                    (xcb_input_key_release_event_t *)ge;

                enum xkb_state_component changed = xkb_state_update_key(
                    xcb_state->xkb_state, kre->detail, XKB_KEY_UP);
                UNUSED(changed);

                xkb_keysym_t keysym = xkb_state_key_get_one_sym(
                    xcb_state->xkb_state, kre->detail);

                // We can use our helper getKeycodeFromKeySym function since
                // the value of keysym is same Scancode sc =
                // Scancode sc = xcb_state->xKeyCodeToScancode[kre->detail];

                Keycode kc = getKeycodeFromKeySym(keysym);
                // Translating the X's keycode to linux's keycode by -8
                // Might not be reliable. Yet to figure out
                inputProcessKey(getScancodeFromLinuxKeycode(kre->detail - 8),
                                kc,
                                getKeymodsFromXKBCommon(xcb_state->xkb_keymap,
                                                        xcb_state->xkb_state),
                                false, false);
            } break;
            case XCB_INPUT_MOTION: {
                xcb_input_motion_event_t *me = (xcb_input_motion_event_t *)ge;
                inputProcessPointerMotion(me->event_x, me->event_y);
                // static f64 ex, ey, rx, ry;
                // sDebug("Motion: device=%d, event=(%.0f,
                // %.0f), "
                //        "root=(%.0f, %.0f)",
                //        me->deviceid, ex - me->event_x,
                //        ey - me->event_y, rx - me->root_x,
                //        ry - me->root_y);
                // ex = me->event_x;
                // ey = me->event_y;
                // rx = me->root_x;
                // ry = me->root_y;
            } break;
            default: {
                sError("Should not be getting some unkown event "
                       "from the XI.");
            } break;
        }
    }
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
    // xcb_get_extension_data(xcb_state->connection)

    // xcb_wait_for_event (blocking) is similar to XNextEvent
    // So use xcb_poll_for_event(non-blocking)
    // ? Do I need to check for !quit
    while (!quit && (event = xcb_poll_for_event(xcb_state->connection))) {
        // should be & with ~0x80. Don't know more
        u8 response_type = event->response_type & ~0x80;

        if (response_type == xcb_state->xkb_event_code) {
            // In every event type of xkb, second member is xkbType of size 8
            // bit. In xcb_generic_event_t pad0 is second member which also
            // have 8 bit size
            switch (event->pad0) {
                case XCB_XKB_NEW_KEYBOARD_NOTIFY: {
                    // sDebug("XkbNewKeyboardNotify");
                    // ? Should call this or not
                    mapKeycodesToScancodes();
                } break;
                case XCB_XKB_MAP_NOTIFY: {
                    // sDebug("XkbMapNotify");
                    // ? Should call this or not
                    // XkbGetUpdatedMap(xlib_state->display,
                    //                  XkbAllMapComponentsMask,
                    //                  xlib_state->xkb_desc);
                    mapKeycodesToScancodes();
                } break;
                case XCB_XKB_NAMES_NOTIFY: {
                    // sDebug("XkbNamesNotify");
                    // ? Should call this or not
                    mapKeycodesToScancodes();
                } break;
                case XCB_XKB_STATE_NOTIFY: {
                    xcb_xkb_state_notify_event_t *state_event =
                        (xcb_xkb_state_notify_event_t *)event;

                    // Update the xkb_state
                    enum xkb_state_component changed = xkb_state_update_mask(
                        xcb_state->xkb_state, state_event->baseMods,
                        state_event->latchedMods, state_event->lockedMods,
                        state_event->baseGroup, state_event->latchedGroup,
                        state_event->lockedGroup);
                    UNUSED(changed);
                } break;
                default:
                    sError("Event type %d from xkb is not being handled?",
                           event->pad0);
            }
            continue;
        }
        switch (response_type) {
            case XCB_GE_GENERIC: {
                handleGenericEvents((xcb_ge_generic_event_t *)event);
            } break;
            case XCB_EXPOSE:
                // TODO:
                break;
            case XCB_CONFIGURE_NOTIFY:
                // TODO:
                break;
            case XCB_CLIENT_MESSAGE: {
                if (((xcb_client_message_event_t *)event)->data.data32[0]
                    == xcb_state->wm_delete_window) {
                    fireEvent(EVENT_CODE_APPLICATION_QUIT, NULL,
                              ((EventContext){0}));
                    quit = true;
                }
            } break;
            case XCB_MAPPING_NOTIFY:
                // Ignore this event since we are handling it in the xkb events
                break;
            default: {
                sTrace("An event is being ignored: Event type: %d",
                       response_type);
            } break;
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
 * @brief Chnage the visibility of the window (xcb implementation).
 *
 * If called with true even if the window is visible, or called with false
 * even if the window is not visible, no error will be generated.
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
 * @brief Set the title of the window (xcb implementation).
 *
 * @param title The title
 *
 * @return Returns true if title was changed successfully.
 */
b8 platformSetWindowTitle(const char *title) {
    sassert_msg(xcb_state, "Windowing system is not initialized?");

    xcb_icccm_set_wm_name(xcb_state->connection, xcb_state->app_window,
                          XCB_ATOM_STRING, 8, sStringLength(title), title);

    return true;
}

/**
 * @brief Get the title of the window (xcb implementation).
 *
 * @return Returns the malloced stirng, user should call sFree.
 */
char *platformGetWindowTitle(void) {
    sassert_msg(xcb_state, "Windowing system is not initialized?");

    xcb_get_property_cookie_t title_cookie =
        xcb_icccm_get_wm_name(xcb_state->connection, xcb_state->app_window);

    xcb_icccm_get_text_property_reply_t *title_reply = NULL;
    if (xcb_icccm_get_wm_name_reply(xcb_state->connection, title_cookie,
                                    title_reply, NULL)) {
        char *ret = sStringCopy(title_reply->name, title_reply->name_len);
        free(title_reply);
        return ret;
    }

    return NULL;
}

#endif
