#include "engine.h"

#include "event.h"
#include "input/input.h"
#include "logger.h"
#include "memory/arenaalloc.h"
#include "memory/memory.h"
#include "platform/window.h"

typedef struct EngineState {
        b8 is_running;
        Application *app;
        SArena arena;
} EngineState;

static EngineState engine_state;

/**
 * @brief We can do things when app quit message is forworded.
 */
b8 applicationQuitEvent(u16 code, void *sender, void *listener,
                        EventContext context) {
    UNUSED(code);
    UNUSED(sender);
    UNUSED(listener);
    UNUSED(context);
    sInfo("Application quit event is recieved");
    // Event is handled
    return true;
}

/**
 * @brief Initialize the engine.
 *
 * This will also initialize subsystems and will call initialize on application.
 *
 * @param app Pointer to the Application instance
 *
 * @return true if engine was initialized successfully.
 *
 * @note If some of the subsystem, like logger, failed to initialize, it is not
 * conidered as failure to initialize engine. In such cases an error message
 * will be given and this function returns true.
 */
b8 initializeEngine(Application *app) {
    if (engine_state.is_running) {
        // Engine is already running, so we had initialized logger.
        sError("Engine is already running, but initializeEngine called again.");
        return false;
    }

    sMemZeroOut(&engine_state, sizeof(EngineState));

    engine_state.app = app;

    if (!initializeMemory()) sError("Failed to initialize memory subsystem");

    u64 event_size, input_size, windowing_size;

    // Calculate the total memory size required for the subsystems
    initializeEvent(&event_size, NULL);
    engine_state.arena.size += event_size;

    initializeInput(&input_size, NULL);
    engine_state.arena.size += input_size;

    initializePlatformWindowing(NULL, &windowing_size, NULL);
    engine_state.arena.size += windowing_size;

    // Create arena of required size
    if (!sArenaCreate(&engine_state.arena)) {
        sFatal("Failed to allocate memory for subsystems");
        return false;
    }

    // Initailze the subsystems
    if (!initializeEvent(&event_size,
                         sArenaAlloc(&engine_state.arena, event_size))) {
        sFatal("Failed to initialize event system");
        return false;
    }

    if (!initializeInput(&input_size,
                         sArenaAlloc(&engine_state.arena, input_size))) {
        sFatal("Failed to initialize the input system");
        return false;
    }

    if (!initializePlatformWindowing(
            &engine_state.app->config, &windowing_size,
            sArenaAlloc(&engine_state.arena, windowing_size))) {
        sFatal("Failed to initialize the windowing system");
        return false;
    }

    engine_state.is_running = true;

    // Should be called at last, i.e., after initializing subsystems
    if (!engine_state.app->initialize(engine_state.app)) {
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
    engine_state.is_running = false;

    if (!unregisterEventListener(EVENT_CODE_APPLICATION_QUIT, NULL,
                                 applicationQuitEvent))
        sError("Failed to unregister from application quit event");

    // Should be called first, i.e., before terminating subsystems
    engine_state.app->terminate(engine_state.app);

    // No need to deallocate memory since our memory system handles it

    shutdownPlatformWindowing();
    shutdownInput();
    shutdownEvent();

    sArenaDestroy(&engine_state.arena);

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
