#include "engine.h"

#include "event.h"
#include "input/input.h"
#include "logger.h"
#include "memory.h"
#include "platform/window.h"

struct EventSystem;
struct WindowingSystem;
struct InputSystem;

typedef struct EngineState {
        b8 is_running;
        Application *app_inst;

        struct EventSystem *event_system;
        struct WindowingSystem *windowing_system;
        struct InputSystem *input_system;
} EngineState;

static EngineState engine_state;

// TODO: Remove this temporary implementation to close the window
/**
 * @brief Temporary solutition to quit application in windows.
 *
 * If on other platforms, we just print this function is called.
 */
b8 applicationQuitEvent(u16 code, void *sender, void *listener,
                        EventContext context) {
    UNUSED(code);
    UNUSED(sender);
    UNUSED(listener);
    UNUSED(context);
    sInfo("Application quit event is recieved");
#ifdef SPLATFORM_OS_WINDOWS
    engine_state.is_running = false;
#endif
    // Event is handled
    return true;
}

/**
 * @brief Initialize the engine.
 *
 * This will also initialize subsystems and will call initialize on application.
 *
 * @param app_inst Pointer to the Application instance
 *
 * @return true if engine was initialized successfully.
 *
 * @note If some of the subsystem, like logger, failed to initialize, it is not
 * conidered as failure to initialize engine. In such cases an error message
 * will be given and this function returns true.
 */
b8 initializeEngine(Application *app_inst) {
    if (engine_state.is_running) {
        // Engine is already running, so we had initialized logger.
        sError("Engine is already running, but initializeEngine called again.");
        return false;
    }

    engine_state.app_inst = app_inst;

    if (!initializeMemory()) {
        sError("Failed to initialize memory subsystem");
    }

    // TODO: Make logger work such that it doesn't need to be initialized.
    // TODO: Or Try to make independent of any systems.
    if (!initializeLogger("log.txt")) {
        // Even if initialize is failed, messages should be logged to the stdout
        // or stderr
        // TODO: Make sure above one is true
        sError("Failed to initialize logger");
    }

    {
        u64 size;
        initializeEvent(&size, NULL);

        engine_state.event_system = sMalloc(size);

        if (!engine_state.event_system) {
            sFatal("Failed to allocate memory for event system");
            return false;
        }

        if (!initializeEvent(&size, engine_state.event_system)) {
            sFatal("Failed to initialize event system");
            return false;
        }
    }

    {
        u64 size;
        initializeInput(&size, NULL);

        engine_state.input_system = sMalloc(size);

        if (!engine_state.input_system) {
            sFatal("Failed to allocate memory for input system");
            return false;
        }

        if (!initializeInput(&size, engine_state.input_system)) {
            sFatal("Failed to initialize the input system");
            return false;
        }
    }

    {
        u64 size;
        initializePlatformWindowing(&engine_state.app_inst->config, &size,
                                    NULL);

        engine_state.windowing_system = sMalloc(size);
        if (!engine_state.windowing_system) {
            sFatal("Failed to allocate memory for event system");
            return false;
        }

        if (!initializePlatformWindowing(&engine_state.app_inst->config, &size,
                                         engine_state.windowing_system)) {
            sFatal("Failed to initialize the windowing system");
            return false;
        }
    }

    engine_state.is_running = true;

    // Should be called at last, i.e., after initializing subsystems
    if (!engine_state.app_inst->initialize(engine_state.app_inst)) {
        sFatal("Application initialization failed!");
        return false;
    }

    if (!registerEventListener(EVENT_CODE_APPLICATION_QUIT, NULL,
                               applicationQuitEvent))
        sError("Failed to register for application quit event");

    return true;
}

/**
 * @brief Shutdown the engine.
 */
void shutdownEngine(void) {
    if (!unregisterEventListener(EVENT_CODE_APPLICATION_QUIT, NULL,
                                 applicationQuitEvent))
        sError("Failed to unregister from application quit event");

    // Should be called first, i.e., before terminating subsystems
    engine_state.app_inst->terminate(engine_state.app_inst);

    // No need to deallocate memory since our memory system handles it

    shutdownPlatformWindowing(engine_state.windowing_system);
    shutdownInput(engine_state.input_system);
    shutdownEvent(engine_state.event_system);
    shutdownLogger();
    shutdownMemory();
}

/**
 * @brief Main loop for the application.
 *
 * @return true if terminated normally, false if terminated abnormally.
 */
b8 engineRun(void) {
    // if engine was not initialized then is_running is false => engineRun
    // failed since not initialized.
    if (!engine_state.is_running) {
        sError("engineRun was called without initializing engine "
               "(engine_state.is_running = false)");
        return false;
    }

    while (engine_state.is_running) {
        if (!platformWindowPumpMessages()) engine_state.is_running = false;
    }

    return true;
}
