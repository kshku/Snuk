#pragma once

#include "application_types.h"
#include "core/engine.h"
#include "core/logger.h"
#include "defines.h"

extern b8 createApplication(Application *app_inst);

int main(int argc, char *argv[]) {
    UNUSED(argc);
    UNUSED(argv);

    Application app_inst = {0};

    if (!createApplication(&app_inst)) {
        sFatal("Could not create game!");
        return -1;
    }

    if (!app_inst.config.name || app_inst.config.width == 0
        || app_inst.config.height == 0) {
        sFatal("Name is not given or width = 0 or height = 0");
        return -1;
    }

    if (!app_inst.initialize || !app_inst.update || !app_inst.render
        || !app_inst.terminate) {
        sFatal("App's function pointer must be assigned!");
        return -1;
    }

    if (!initializeEngine(&app_inst)) {
        sFatal("Failed to initialize engine!");
        shutdownEngine();
        return -1;
    }

    if (!engineRun()) {
        sError("engineRun returned false");
    }

    shutdownEngine();

    return 0;
}
