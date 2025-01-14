#pragma once

#include "application_types.h"
#include "core/engine.h"
#include "core/logger.h"
#include "defines.h"

// extern b8 createApplication(Application *app_inst);

extern Application app;

int main(int argc, char *argv[]) {
    UNUSED(argc);
    UNUSED(argv);

    // Application app_inst = {0};

    // if (!createApplication(&app_inst)) {
    //     sFatal("Could not create game!");
    //     return -1;
    // }

    if (!app.config.name || app.config.width == 0 || app.config.height == 0) {
        sFatal("Name is not given or width = 0 or height = 0");
        return -1;
    }

    if (!app.initialize || !app.update || !app.render || !app.terminate) {
        sFatal("App's function pointer must be assigned!");
        return -1;
    }

    if (!initializeEngine(&app)) {
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
