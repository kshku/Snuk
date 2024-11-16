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
        SFATAL("Could not create game!");
        return -1;
    }

    if (!app_inst.initialize || !app_inst.update || !app_inst.render
        || !app_inst.terminate) {
        SFATAL("App's function pointer must be assigned!");
        return -1;
    }

    if (!initializeEngine(&app_inst)) {
        SFATAL("Failed to initialize engine!");
        shutdownEngine();
        return -1;
    }

    if (!engineRun()) {
        SERROR("engineRun returned false");
    }

    shutdownEngine();

    return 0;
}
