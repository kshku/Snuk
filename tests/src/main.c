#include <core/logger.h>
#include <core/memory.h>
#include <entry.h>

#include "test_manager.h"

// NOTE: Include headers for tests here
#include "tests/darray.h"
#include "tests/memory.h"
#include "tests/sstring.h"

b8 tests_init(Application *app_inst) {
    sInfo("Before registering tests");
    sLogMemState();

    // NOTE: Register tests here
    app_inst->state = core_memory_register_tests(app_inst->state);
    app_inst->state = ds_darray_register_tests(app_inst->state);
    app_inst->state = core_sstring_register_tests(app_inst->state);
    // -------------------

    sInfo("After registering tests");
    sLogMemState();

    testManagerRun(app_inst->state);

    return true;
}

b8 tests_update(Application *app_inst, f32 delta_time) {
    UNUSED(app_inst);
    UNUSED(delta_time);
    sTrace("tests_update is called");
    return false;
}

b8 tests_render(Application *app_inst, f32 delta_time) {
    UNUSED(app_inst);
    UNUSED(delta_time);
    sTrace("tests_render is called");
    return false;
}

void tests_terminate(Application *app_inst) {
    shutdownTestManager(app_inst->state);
}

b8 createApplication(Application *app_inst) {
    app_inst->config.name = "Snuk Tests";
    app_inst->config.x = 50;
    app_inst->config.y = 50;
    app_inst->config.width = 600;
    app_inst->config.height = 400;

    app_inst->initialize = tests_init;
    app_inst->update = tests_update;
    app_inst->render = tests_render;
    app_inst->terminate = tests_terminate;

    app_inst->state = (void *)initializeTestManager();

    return true;
}
