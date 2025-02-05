#include <core/logger.h>
#include <core/memory/memory.h>
#include <entry.h>

#include "test_manager.h"

// NOTE: Include headers for tests here
#include "tests/atomic.h"
#include "tests/darray.h"
#include "tests/log.h"
#include "tests/memory.h"
#include "tests/sstring.h"

b8 tests_init(Application *app_inst) {
    app_inst->data = (void *)initializeTestManager();

    sInfo("Before registering tests");
    sMemLogState();

    // NOTE: Register tests here
    app_inst->data = log_register_tests(app_inst->data);
    app_inst->data = core_memory_register_tests(app_inst->data);
    app_inst->data = ds_darray_register_tests(app_inst->data);
    app_inst->data = core_sstring_register_tests(app_inst->data);
    app_inst->data = atomic_tests(app_inst->data);
    // -------------------

    // Manual break point test
    // DEBUG_BREAK;

    sInfo("After registering tests");
    sMemLogState();

    testManagerRun(app_inst->data);

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
    shutdownTestManager(app_inst->data);
}

// b8 createApplication(Application *app_inst) {
//     app_inst->config.name = "Snuk Tests";
//     app_inst->config.x = 50;
//     app_inst->config.y = 50;
//     app_inst->config.width = 600;
//     app_inst->config.height = 400;

//     app_inst->initialize = tests_init;
//     app_inst->update = tests_update;
//     app_inst->render = tests_render;
//     app_inst->terminate = tests_terminate;

//     app_inst->data = (void *)initializeTestManager();

//     return true;
// }

Application app = {
    .config =
        {.name = "Snuk Tests", .x = 50, .y = 50, .width = 600, .height = 400},
    .data = NULL,
    .initialize = tests_init,
    .render = tests_render,
    .update = tests_update,
    .terminate = tests_terminate
};
