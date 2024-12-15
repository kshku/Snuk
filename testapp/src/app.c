#include <core/event.h>
#include <core/logger.h>
#include <entry.h>

typedef struct AppState {
} AppState;

b8 key_press_logger(u16 code, void *sender, void *listener,
                    EventContext context);
b8 key_release_logger(u16 code, void *sender, void *listener,
                      EventContext context);

b8 button_press_logger(u16 code, void *sender, void *listener,
                       EventContext context);
b8 button_release_logger(u16 code, void *sender, void *listener,
                         EventContext context);

b8 scroll_logger(u16 code, void *sender, void *listener, EventContext context);

b8 appInitialize(Application *app_inst) {
    UNUSED(app_inst);
    sInfo("appInitialize is called");

    if (!registerEventListener(EVENT_CODE_KEY_PRESS, NULL, key_press_logger))
        return false;

    if (!registerEventListener(EVENT_CODE_KEY_RELEASE, NULL,
                               key_release_logger))
        return false;

    if (!registerEventListener(EVENT_CODE_BUTTON_PRESS, NULL,
                               button_press_logger))
        return false;

    if (!registerEventListener(EVENT_CODE_BUTTON_RELEASE, NULL,
                               button_release_logger))
        return false;

    if (!registerEventListener(EVENT_CODE_SCROLL, NULL, scroll_logger))
        return false;

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
    unregisterEventListener(EVENT_CODE_KEY_PRESS, NULL, key_press_logger);
    unregisterEventListener(EVENT_CODE_KEY_RELEASE, NULL, key_release_logger);
    unregisterEventListener(EVENT_CODE_BUTTON_PRESS, NULL, button_press_logger);
    unregisterEventListener(EVENT_CODE_BUTTON_RELEASE, NULL,
                            button_release_logger);
    unregisterEventListener(EVENT_CODE_SCROLL, NULL, scroll_logger);
}

b8 createApplication(Application *app_inst) {
    app_inst->config.name = "Snuk Testapp";
    app_inst->config.x = 50;
    app_inst->config.y = 50;
    app_inst->config.width = 600;
    app_inst->config.height = 400;

    app_inst->initialize = appInitialize;
    app_inst->update = appUpdate;
    app_inst->render = appRender;
    app_inst->terminate = appTerminate;

    app_inst->state = NULL;

    return true;
}

b8 key_press_logger(u16 code, void *sender, void *listener,
                    EventContext context) {
    UNUSED(sender);
    UNUSED(listener);
    if (code != EVENT_CODE_KEY_PRESS) sError("Getting unregisterd events!");

    u32 key = context.data.u32[0];

    sInfo("Pressed '%d'", key);

    return false;
}

b8 key_release_logger(u16 code, void *sender, void *listener,
                      EventContext context) {
    UNUSED(sender);
    UNUSED(listener);
    if (code != EVENT_CODE_KEY_RELEASE) sError("Getting unregisterd events!");

    u32 key = context.data.u32[0];

    sInfo("Released '%d'", key);

    return false;
}

b8 button_press_logger(u16 code, void *sender, void *listener,
                       EventContext context) {
    UNUSED(sender);
    UNUSED(listener);
    if (code != EVENT_CODE_BUTTON_PRESS) sError("Getting unregisterd events!");

    u16 button = context.data.u16[0];

    sInfo("Pressed '%d'", button);

    return false;
}

b8 button_release_logger(u16 code, void *sender, void *listener,
                         EventContext context) {
    UNUSED(sender);
    UNUSED(listener);
    if (code != EVENT_CODE_BUTTON_RELEASE)
        sError("Getting unregisterd events!");

    u16 button = context.data.u16[0];

    sInfo("Released '%d'", button);

    return false;
}

b8 scroll_logger(u16 code, void *sender, void *listener, EventContext context) {
    UNUSED(sender);
    UNUSED(listener);
    if (code != EVENT_CODE_SCROLL) sError("Getting unregistered events!");

    u16 direction = context.data.u16[0];

    sInfo("Direction '%d'", direction);

    return false;
}
