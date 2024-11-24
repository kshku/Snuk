#include "../../window.h"

#ifdef SPLATFORM_WINDOWING_WAYLAND
// Documentation used
// https://wayland-book.com/

// TODO: Implement windowing system for wayland

    #include <wayland-client.h>

    #include "core/assertions.h"
    #include "core/logger.h"

    // Todo: Write string library
    #include <string.h>

typedef struct WaylandState {
        struct wl_display *display;
        struct wl_registry *registry;
        struct wl_compositor *compositor;
        struct wl_surface *surface;
        struct wl_shm *shm;
} WaylandState;

static WaylandState *wayland_state;

/**
 * @brief global registry listener [INTERNAL FUNCTION].
 *
 * @param data
 * @param registry
 * @param name
 * @param interface
 * @param version
 */
void registryHandleGlobal(void *data, struct wl_registry *registery, u32 name,
                          const char *interface, u32 version) {
    sDebug("Interface: '%s', version: %d, name %d", interface, version, name);

    if (!strcmp(interface, wl_compositor_interface.name))
        wayland_state->compositor = wl_registry_bind(
            registery, name, &wl_compositor_interface, version);

    if (!strcmp(interface, wl_shm_interface.name))
        wayland_state->shm =
            wl_registry_bind(registery, name, &wl_shm_interface, version);
}

/**
 * @brief global registry remove listener [INTERNAL FUNCTION]
 *
 * @param data
 * @param registry
 * @param name
 */
void registryHandleGlobalRemove(void *data, struct wl_registry *registry,
                                u32 name) {
}

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

    // TODO: Error handling
    // Connect to wayland server
    wayland_state->display = wl_display_connect(NULL);
    if (!wayland_state->display) {
        sError("Failed to connect to wayland server");
        return false;
    }

    // Add registry listeners
    wayland_state->registry = wl_display_get_registry(wayland_state->display);
    const struct wl_registry_listener registry_listener = {
        .global = registryHandleGlobal,
        .global_remove = registryHandleGlobalRemove};
    wl_registry_add_listener(wayland_state->registry, &registry_listener, NULL);

    // Do the round trip
    wl_display_roundtrip(wayland_state->display);

    wayland_state->surface =
        wl_compositor_create_surface(wayland_state->compositor);

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

    if (wayland_state->display) wl_display_disconnect(wayland_state->display);
}

/**
 * @brief Loop through all the messages and fire the corresponding events.
 *
 * @return Returns false if application quit was recieved else true.
 */
b8 platformWindowPumpMessages(void) {
    sassert_msg(wayland_state, "Windowing system is not initialized?");
    return false;
}

b8 platformWindowCreate() {
    // TODO:
    return false;
}

void platformWindowDestroy() {
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
}

/**
 * @brief Set the title of the window (wayland implementation).
 *
 * @param title The title
 *
 * @return Returns true if title was changed successfully.
 */
b8 platformSetWindowTitle(const char *title) {
    sassert_msg(wayland_state, "Windowing system is not initialized?");
}

/**
 * @brief Get the title of the window (wayland implementation).
 *
 * @param[out] title Title will be copied to this
 * @param size Maximum size can be written to the title
 *
 * @return Returns true if title was set successfully, else false.
 */
b8 platformGetWindowTitle(char *title, u64 size) {
    sassert_msg(wayland_state, "Windowing system is not initialized?");
}

#endif
