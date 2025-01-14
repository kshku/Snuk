#pragma once

#include "defines.h"

typedef struct MainWindowConfig {
        const c8 *name;
        i32 x, y;
        u32 width, height;
} MainWindowConfig;

typedef struct Application {
        // Main window configurations
        MainWindowConfig config;

        // Function pointer to app's initialize function
        b8 (*initialize)(struct Application *app_inst);

        // Function pointer to app's update function
        b8 (*update)(struct Application *app_inst, f32 detla_time);

        // Function pointer to app's render function
        b8 (*render)(struct Application *app_inst, f32 delta_time);

        // Function pointer to app's terminate function
        void (*terminate)(struct Application *app_inst);

        // Any arbitary data that app might want to store here so that it can
        // get it through the app_inst parameter in the above functions
        // App has to manage this data.
        void *data;
} Application;
