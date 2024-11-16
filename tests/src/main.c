#include <core/logger.h>
#include <entry.h>

#include "test_manager.h"

// NOTE: Include headers for tests here
#include "tests/memory.h"

b8 tests_init(Application *app_inst) {
    UNUSED(app_inst);

    initializeTestManager();

    // NOTE: Register tests here
    core_memory_register_tests();

    // -------------------

    testManagerRun();

    shutdownTestManager();
    return false;
}

b8 tests_update(Application *app_inst, f32 delta_time) {
    UNUSED(app_inst);
    UNUSED(delta_time);
    STRACE("tests_update is called");
    return false;
}

b8 tests_render(Application *app_inst, f32 delta_time) {
    UNUSED(app_inst);
    UNUSED(delta_time);
    STRACE("tests_render is called");
    return false;
}

void tests_terminate(Application *app_inst) {
    UNUSED(app_inst);
    STRACE("tests_terminate is called");
}

b8 createApplication(Application *app_inst) {
    app_inst->initialize = tests_init;
    app_inst->update = tests_update;
    app_inst->render = tests_render;
    app_inst->terminate = tests_terminate;

    app_inst->state = NULL;

    return true;
}
