#include <core/logger.h>
#include <entry.h>

typedef struct AppState {
} AppState;

b8 appInitialize(Application *app_inst) {
    UNUSED(app_inst);
    sInfo("appInitialize is called");
    return true;
}

b8 appUpdate(Application *app_inst, f32 delta_time) {
    UNUSED(app_inst);
    UNUSED(delta_time);
    sInfo("appUpdate is called");
    return true;
}

b8 appRender(Application *app_inst, f32 delta_time) {
    UNUSED(app_inst);
    UNUSED(delta_time);
    sInfo("appRender is called");
    return true;
}

void appTerminate(Application *app_inst) {
    UNUSED(app_inst);
    sInfo("appTerminate is called");
}

b8 createApplication(Application *app_inst) {
    app_inst->initialize = appInitialize;
    app_inst->update = appUpdate;
    app_inst->render = appRender;
    app_inst->terminate = appTerminate;

    app_inst->state = NULL;

    return true;
}
