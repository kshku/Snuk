#include "engine.h"

#include "event.h"
#include "logger.h"
#include "memory.h"

struct EventSystem;

typedef struct EngineState {
        b8 is_running;
        Application *app_inst;

        struct EventSystem *event_system;
} EngineState;

static EngineState engine_state;

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

        if (!initializeEvent(&size, engine_state.event_system)) {
            sFatal("Failed to initialize event system");
            return false;
        }
    }

    engine_state.is_running = true;

    // Should be called at last, i.e., after initializing subsystems
    if (!engine_state.app_inst->initialize(engine_state.app_inst)) {
        sFatal("Application initialization failed!");
        return false;
    }

    return true;
}

/**
 * @brief Shutdown the engine.
 */
void shutdownEngine(void) {
    // Should be called first, i.e., before terminating subsystems
    engine_state.app_inst->terminate(engine_state.app_inst);

    // No need to deallocate memory since our memory system handles it

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
    b8 ret_val = engine_state.is_running;
    if (!ret_val) {
        sError("engineRun was called without initializing engine "
               "(engine_state.is_running = false)");
        return false;
    }

    while (engine_state.is_running) {
        if (!engine_state.app_inst->update(engine_state.app_inst, (f32)0)) {
            sFatal("Application update failed!");
            ret_val = false;
            // TODO: Yet to decide how to terminate main loop
            engine_state.is_running = false;
            break;
        }

        engine_state.is_running = false;
    }

    return ret_val;
}
