#include "../../window.h"

#ifdef SPLATFORM_WINDOWING_WAYLAND
// Documentation used
// https://wayland-book.com/

// TODO: Implement windowing system for wayland
    #define _POSIX_C_SOURCE_200112L
    #include <errno.h>
    #include <fcntl.h>
    #include <sys/mman.h>
    #include <unistd.h>
    #include <wayland-client.h>
    #include <wayland-protocols/xdg-shell-enum.h>

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
        struct wl_shm_pool *shm_pool;
        struct wl_buffer *buffer;
        struct xdg_surface *xdg_surface;
        struct xdg_toplevel *xdg_toplevel;

        i32 shm_fd;
        i32 width, height, stride, shm_pool_size;
        u8 *shm_pool_data;
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

    // if (!strcmp(interface, ))
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
 * @brief Destroy the buffer
 */
void bufferRelease(void *data, struct wl_buffer *buffer) {
    wl_buffer_destroy(buffer);
}

/**
 * @brief Create shm file
 */
i32 create_shm_file(u64 size) {
    const char *name = "/wl_shm_file_snuk";

    i32 fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);

    if (fd < 0) {
        if (errno == EEXIST) sError("Failed to create shm file");
        return -1;
    }

    shm_unlink(name);
    i32 fdr;
    do {
        fdr = ftruncate(fd, size);
    } while (fdr < 0 && errno == EINTR);

    if (fdr < 0) {
        close(fd);
        return -1;
    }

    return fdr;
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

    // Shared memory buffers
    wayland_state->width = config->width;
    wayland_state->height = config->height;
    wayland_state->stride = wayland_state->width * 4;  // 4 bytes per pixel
    wayland_state->shm_pool_size = wayland_state->height * wayland_state->stride
                                 * 2;  // 2 buffers for double buffering

    wayland_state->shm_fd = create_shm_file(wayland_state->shm_pool_size);
    wayland_state->shm_pool_data =
        mmap(NULL, wayland_state->shm_pool_size, PROT_READ | PROT_WRITE,
             MAP_SHARED, wayland_state->shm_fd, 0);

    // i32 index = 0;
    // i32 offset = wayland_state->height * wayland_state->stride * index;
    wayland_state->buffer = wl_shm_pool_create_buffer(
        wayland_state->shm_pool, 0, wayland_state->width, wayland_state->height,
        wayland_state->stride, WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(wayland_state->shm_pool);
    close(wayland_state->shm_fd);

    uint32_t *pixels = (uint32_t *)&(wayland_state->shm_pool_data);

    // Solid white color
    // memset(pixels, 0, width * height * 4);

    // Checker board
    for (int y = 0; y < wayland_state->height; ++y) {
        for (int x = 0; x < wayland_state->width; ++x) {
            if ((x + y / 8 * 8) % 16 < 8) {
                pixels[y * wayland_state->width + x] = 0xFF666666;
            } else {
                pixels[y * wayland_state->width + x] = 0xFFEEEEEE;
            }
        }
    }

    munmap(wayland_state->shm_pool_data, wayland_state->shm_pool_size);

    const struct wl_buffer_listener buffer_listener = {.release =
                                                           bufferRelease};
    wl_buffer_add_listener(wayland_state->buffer, &buffer_listener, NULL);

    // Attach buffer to surface, mark damaged, commit
    wl_surface_attach(
        wayland_state->surface, wayland_state->buffer, config->x,
        config->y);  // I think it is taking the x and y pos of window
    wl_surface_damage(wayland_state->surface, 0, 0, wayland_state->width,
                      wayland_state->height);
    wl_surface_commit(wayland_state->surface);

    // u32 len;
    // for (len = 0; config->name[len]; ++len);
    // const char *append = " - X11(Xlib)";
    // char *app_name = (char *)sMalloc(len + 14);
    // sMemCopy((void *)app_name, (void *)config->name, len);
    // sMemCopy((((void *)app_name) + len), (void *)append, 14);
    // if (!platformSetWindowTitle(app_name))
    //     sError("Couldn't set the window title");
    // sFree(app_name);

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
