#include "../../window.h"

#ifdef SPLATFORM_WINDOWING_WIN32

    #include <Windows.h>
    #include <windowsx.h>  // Param input extraction

    #include "core/assertions.h"
    #include "core/event.h"
    #include "core/logger.h"
    #include "core/memory.h"
    #include "core/sstring.h"

typedef struct Win32State {
        HINSTANCE h_instance;
        HWND hwnd;
} Win32State;

static Win32State *win32_state;

/**
 * @brief Process the messages [INTERNAL FUNCTION]
 *
 * @param hwnd
 * @param msg
 * @param w_param
 * @param l_param
 *
 * @return
 */
LRESULT CALLBACK win32ProcessMessage(HWND hwnd, u32 msg, WPARAM w_param,
                                     LPARAM l_param) {
    // To know more about why return that perticular value see documentation
    switch (msg) {
        case WM_ERASEBKGND:
            // Notify the OS that erasing will be handled by the application to
            // prevent flicker.
            return 1;
        case WM_CLOSE:
            // Fire application quit event
            if (!fireEvent(EVENT_CODE_APPLICATION_QUIT, NULL,
                           ((EventContext){0}))) {
                sError("Expected someone to handle Application quit event");
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            // Get the updated size
            // RECT r;
            // GetClientRect(hwnd, &r);
            // u32 width = r.right - r.left;
            // u32 height = r.bottom - r.top;
            // TODO: Fire window resize event
            break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            // TODO
            break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            // TODO
            break;
        case WM_MOUSEMOVE:
            // i32 x_pos = GET_X_LPARAM(l_param);
            // i32 y_pos = GET_Y_LPARAM(l_param);
            // TODO:
            break;
        case WM_MOUSEWHEEL:
            // TODO:
            break;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
            break;
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            break;
        default:
            break;
    }

    return DefWindowProcA(hwnd, msg, w_param, l_param);
}

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

    // NOTE: Just following from Kohi series. Not interested now in going
    // NOTE: through the windows documentation or search topics to write this
    // NOTE: Will look at the docs and may rewrite these things in future.

    // Load this application
    win32_state->h_instance = GetModuleHandleA(NULL);
    if (!win32_state->h_instance) {
        sError("Failed to load this application's handle");
        return false;
    }

    // Setup window class
    WNDCLASSA wc = {.style = CS_DBLCLKS,  // Get double clicks
                    .lpfnWndProc = win32ProcessMessage,
                    .cbClsExtra = 0,
                    .cbWndExtra = 0,
                    .hInstance = win32_state->h_instance,
                    .hIcon = LoadIcon(win32_state->h_instance, IDI_APPLICATION),
                    .hCursor = LoadCursor(
                        NULL, IDC_ARROW),  // NULL -> Manage cursor manually
                    .hbrBackground = NULL,  // Transparent
                    .lpszClassName = config->name};

    // Register
    if (!RegisterClassA(&wc)) {
        MessageBoxA(NULL, "Window registration failed!", "Error!",
                    MB_ICONEXCLAMATION | MB_OK);
        sError("Failed to register window");
        return false;
    }

    u32 client_x = config->x;
    u32 client_y = config->y;
    u32 client_width = config->width;
    u32 client_height = config->height;

    u32 window_x = client_x;
    u32 window_y = client_y;
    u32 window_width = client_width;
    u32 window_height = client_height;

    u32 window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
    u32 window_ex_style = WS_EX_APPWINDOW;

    window_style |= WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME;

    // Obtain the size of the boarder
    RECT border_rect = {0};
    AdjustWindowRectEx(&border_rect, window_style, FALSE, window_ex_style);

    window_x += border_rect.left;
    window_y += border_rect.top;
    window_width += border_rect.right - border_rect.left;
    window_height += border_rect.bottom - border_rect.top;

    win32_state->hwnd = CreateWindowExA(
        window_ex_style, config->name, config->name, window_style, window_x,
        window_y, window_width, window_height, NULL, NULL,
        win32_state->h_instance, NULL);

    if (!win32_state->hwnd) {
        MessageBoxA(NULL, "Window creation failed!", "Error!",
                    MB_ICONEXCLAMATION | MB_OK);
        sError("Window creation failed");
        return false;
    }

    char *app_name = sStringConcat(config->name, " - Win32", 0, 8, NULL);
    if (!platformSetWindowTitle(app_name))
        sError("Couldn't set the window title");
    sFree(app_name);

    if (!platformSetWindowVisible(true)) sError("Failed to show the window");

    return true;
}

/**
 * @brief Implementation of Win32.
 *
 * @param state Pointer to the allocated memory
 */
void shutdownPlatformWindowing(void *state) {
    sassert_msg(win32_state,
                "Shutting down windowing system twice or not initialized?");
    UNUSED(state);

    if (win32_state->hwnd) DestroyWindow(win32_state->hwnd);
}

/**
 * @brief Loop through all the messages and fire the corresponding events.
 *
 * @return Returns false if application quit was recieved else true.
 */
b8 platformWindowPumpMessages(void) {
    sassert_msg(win32_state, "Windowing system is not initialized?");

    MSG message;

    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    return true;
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
 * If called with true even if the window is visible, or called with false even
 * if the window is not visible, no error will be generated.
 *
 * @param visibility if true make window visible
 *
 * @return Returns true if changes were made successfully.
 */
b8 platformSetWindowVisible(b8 visible) {
    sassert_msg(win32_state, "Windowing system is not initialized?");
    if (visible) {
        // TODO:
        b8 should_activate = 1;
        i32 show_window_command_flags =
            should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;
        ShowWindow(win32_state->hwnd, show_window_command_flags);
    } else {
        ShowWindow(win32_state->hwnd, SW_HIDE);
    }
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
    if (!SetWindowTextA(win32_state->hwnd, title)) return false;
    return true;
}

/**
 * @brief Get the title of the window (Win32 implementation).
 *
 * @return Returns the malloced stirng, user should call sFree.
 */
char *platformGetWindowTitle(void) {
    const u64 size = 256;
    char *title = sMalloc(size * sizeof(char));
    if (!GetWindowTextA(win32_state->hwnd, title, size)) return NULL;
    return title;
}

#endif
