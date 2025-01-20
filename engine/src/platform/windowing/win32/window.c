#include "../../window.h"

// TODO: UNICODE and ASCII things
// Check which one and use functions (mainly in setting window tiltes and all)
// TODO: Error handling

#ifdef SPLATFORM_WINDOWING_WIN32
    // Include this before including the Windows specific header files
    // Else names like DELETE will collide
    #include "input/input.h"

    // Windows specific
    #include <Windows.h>
    #include <windowsx.h>  // Param input extraction

    #include "core/assertions.h"
    #include "core/event.h"
    #include "core/logger.h"
    #include "core/memory/memory.h"
    #include "core/sstring.h"
    #include "input_helper.h"

typedef struct Win32State {
        HINSTANCE h_instance;
        HWND hwnd;
        // c16 *app_name;
        b8 quit;
} Win32State;

static Win32State *win32_state;

// /**
//  * @brief Convert normal string to wide string.
//  *
//  * @param str String to convert
//  *
//  * @return Converted string.
//  */
// c16 *convertToWideString(const char *str) {
//     u64 len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
//     c16 *converted = sMalloc(len * sizeof(c16));
//     MultiByteToWideChar(CP_UTF8, 0, str, -1, converted, len);

//     len = NormalizeString(NormalizationC, converted, -1, NULL, 0);
//     c16 *normalized = sMalloc(len * sizeof(c16));
//     NormalizeString(NormalizationC, converted, -1, normalized, len);

//     sFree(converted);

//     return normalized;
// }

// /**
//  * @brief Convert wide string to normal string.
//  *
//  * @param str String to convert
//  *
//  * @return Converted string.
//  */
// char *convertToNormalString(const c16 *str) {
//     u64 len = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
//     char *converted = sMalloc(len * sizeof(char));
//     WideCharToMultiByte(CP_UTF8, 0, str, -1, converted, len, NULL, NULL);

//     return converted;
// }

/**
 * @brief Window procedure method (callback function) [INTERNAL FUNCTION].
 *
 * @param hwnd
 * @param u_msg
 * @param w_param
 * @param l_param
 *
 * @return
 */
LRESULT CALLBACK windowProcedure(HWND hwnd, UINT u_msg, WPARAM w_param,
                                 LPARAM l_param) {
    switch (u_msg) {
        case WM_CLOSE:
            // NOTE: DefWindowProc will handle this and Destroy the window
            win32_state->quit = true;
            fireEvent(EVENT_CODE_APPLICATION_QUIT, NULL, ((EventContext){0}));
            break;
        case WM_DESTROY:
            // I know multiple times but still
            win32_state->quit = true;
            PostQuitMessage(0);
            return 0;

        // Mouse events
        case WM_MOUSEMOVE:
            inputProcessPointerMotion(GET_X_LPARAM(l_param),
                                      GET_Y_LPARAM(l_param));
            return 0;
        case WM_LBUTTONDBLCLK:
            // Double click left button
            break;
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
            inputProcessButton(BUTTON_LEFT, GET_X_LPARAM(l_param),
                               GET_Y_LPARAM(l_param), getKeymods(),
                               u_msg == WM_LBUTTONDOWN);
            return 0;
        case WM_MBUTTONDBLCLK:
            // Dobule click right button
            break;
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
            inputProcessButton(BUTTON_MIDDLE, GET_X_LPARAM(l_param),
                               GET_Y_LPARAM(l_param), getKeymods(),
                               u_msg == WM_MBUTTONDOWN);
            return 0;
        case WM_RBUTTONDBLCLK:
            // Double click right button
            break;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
            inputProcessButton(BUTTON_RIGHT, GET_X_LPARAM(l_param),
                               GET_Y_LPARAM(l_param), getKeymods(),
                               u_msg == WM_RBUTTONDOWN);
            return 0;
        case WM_MOUSEWHEEL:
            inputProcessScroll(
                GET_WHEEL_DELTA_WPARAM(w_param) > 0 ? SCROLL_UP : SCROLL_DOWN,
                getKeymods(), 1);
            return 0;
        case WM_MOUSEHWHEEL:
            inputProcessScroll(GET_WHEEL_DELTA_WPARAM(w_param) > 0
                                   ? SCROLL_RIGHT
                                   : SCROLL_LEFT,
                               getKeymods(), 1);
            return 0;

        // Keyboard events
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            // How to handle the key repeat?
            WORD key_flags = HIWORD(l_param);
            BYTE scancode = LOBYTE(key_flags);
            b8 is_extended = key_flags & KF_EXTENDED;
            b8 pressed = (u_msg == WM_KEYDOWN) || (u_msg == WM_SYSKEYDOWN);
            u16 virtual_keycode = LOWORD(w_param);

            // Update the keymod
            updateKeymodsState(virtual_keycode, pressed);

            // Don't know why but sometimes the key press event is not
            // triggered, but release is triggered. Is it only for me?
            inputProcessKey(
                scan1MakeToScancode(scancode, is_extended),
                virtualKeyCodeToKeycode(virtual_keycode, scancode, is_extended),
                getKeymods(), pressed, (LOWORD(l_param) > 0));
        }
            return 0;

        default:
            break;
    }

    return DefWindowProc(hwnd, u_msg, w_param, l_param);
}

// Hooks
// HHOOK hook;

// Used to test the virtual keycodes and scancodes manually.
// LRESULT CALLBACK keyboardhook(i32 n_code, WPARAM w_param, LPARAM l_param) {
//     if (n_code == HC_ACTION) {
//         sDebug("VK = '%u'", ((KBDLLHOOKSTRUCT *)l_param)->vkCode);
//         return 1;  // Block the event
//     }
//     return CallNextHookEx(hook, n_code, w_param, l_param);
// }

/**
 * @brief Implementation for Win32.
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
    sassert_msg(!win32_state, "Initializing the Windowing system twice?");

    *size = sizeof(Win32State);

    if (!state) return false;

    win32_state = (Win32State *)state;

    sMemZeroOut(win32_state, sizeof(Win32State));

    // Retrive the module handle
    win32_state->h_instance = GetModuleHandle(NULL);
    if (!win32_state->h_instance) {
        sError("Failed to get the handle");
        return false;
    }

    // win32_state->app_name = convertToWideString(config->name);

    WNDCLASS window_class = {.lpfnWndProc = windowProcedure,
                             .hInstance = win32_state->h_instance,
                             .lpszClassName = config->name,
                             .style = CS_DBLCLKS};

    if (!RegisterClass(&window_class)) {
        sError("Failed to register the window class");
        return false;
    }

    // RECT boarder_rect = {0};
    RECT boarder_rect = {.left = config->x,
                         .right = config->x + config->width,
                         .top = config->y,
                         .bottom = config->y + config->height};

    AdjustWindowRectEx(&boarder_rect, WS_OVERLAPPEDWINDOW, false,
                       WS_EX_APPWINDOW);

    i64 window_x = boarder_rect.left;
    i64 window_y = boarder_rect.top;
    i64 window_width = boarder_rect.right - boarder_rect.left;
    i64 window_height = boarder_rect.bottom - boarder_rect.top;

    char *name = sStringConcat(config->name, " - Win32", 0, 8, NULL);
    // c16 *app_name = convertToWideString(name);

    win32_state->hwnd =
        CreateWindowEx(WS_EX_APPWINDOW, config->name, name, WS_OVERLAPPEDWINDOW,
                       window_x, window_y, window_width, window_height, NULL,
                       NULL, win32_state->h_instance, NULL);
    sFree(name);

    if (!win32_state->hwnd) {
        sError("Failed to create window");
        return false;
    }

    // An application should specify this flag when displaying the window for
    // the first time. Documentation says. So not calling
    // paltformSetWindowVisible
    ShowWindow(win32_state->hwnd, SW_SHOWNORMAL);

    // hook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardhook, NULL, 0);

    syncKeymodsState();

    return true;
}

/**
 * @brief Implementation of Win32.
 */
void shutdownPlatformWindowing(void) {
    sassert_msg(win32_state,
                "Shutting down windowing system twice or not initialized?");

    // UnhookWindowsHookEx(hook);

    // if (win32_state->app_name) sFree(win32_state->app_name);

    // NOTE: Window procedure itself will destroy the window
}

/**
 * @brief Loop through all the messages and fire the corresponding events.
 *
 * @return Returns false if application quit was recieved else true.
 */
b8 platformWindowPumpMessages(void) {
    sassert_msg(win32_state, "Windowing system is not initialized?");

    MSG message;
    while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    return !win32_state->quit;
}

b8 platformWindowCreate() {
    // TODO:
    return false;
}

void platformWindowDestroy() {
    // TODO:
}

/**
 * @brief Chnage the visibility of the window (Win32 implementation).
 *
 * If called with true even if the window is visible, or called with false
 * even if the window is not visible, no error will be generated.
 *
 * @param visibility if true make window visible
 *
 * @return Returns true if changes were made successfully.
 */
b8 platformSetWindowVisible(b8 visible) {
    UNUSED(visible);
    sassert_msg(win32_state, "Windowing system is not initialized?");
    ShowWindow(win32_state->hwnd, visible ? SW_SHOW : SW_HIDE);
    return true;
}

/**
 * @brief Set the title of the window (Win32 implementation).
 *
 * @param title The title
 *
 * @return Returns true if title was changed successfully.
 */
b8 platformSetWindowTitle(const char *title) {
    sassert_msg(win32_state, "Windowing system is not initialized?");
    // c16 *w_title = convertToWideString(title);
    if (!SetWindowText(win32_state->hwnd, title)) {
        return false;
    }
    return true;
}

/**
 * @brief Get the title of the window (Win32 implementation).
 *
 * @return Returns the malloced stirng, user should call sFree.
 */
char *platformGetWindowTitle(void) {
    sassert_msg(win32_state, "Windowing system is not initialized?");
    const u64 size = GetWindowTextLength(win32_state->hwnd);
    char *title = sMalloc(size * sizeof(char));
    if (!GetWindowText(win32_state->hwnd, title, size)) {
        sFree(title);
        return NULL;
    }
    return title;
}

#endif
