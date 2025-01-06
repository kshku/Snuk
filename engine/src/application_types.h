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

        // App specific state. Created and managed by app.
        // TODO: Might want to remove it from here since memory subsystem will
        // TODO: be started after engine is initialized.
        // NOTE: Another option is, if you want to use memroy subsystem
        // NOTE: initialize this when initialize function is called
        // NOTE: and for that we can change createApplication so that it has 4
        // NOTE: parameters which are function pointers that will be assigned to
        // NOTE: above functions
        void *state;
} Application;
