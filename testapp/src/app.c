#include <core/event.h>
#include <core/logger.h>
#include <core/memory/memory.h>
#include <entry.h>

typedef struct AppState {
        b8 initialized;
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

// b8 createApplication(Application *app_inst) {
//     app_inst->config.name = "Snuk Testapp";
//     app_inst->config.x = 50;
//     app_inst->config.y = 50;
//     app_inst->config.width = 600;
//     app_inst->config.height = 400;

//     app_inst->initialize = appInitialize;
//     app_inst->update = appUpdate;
//     app_inst->render = appRender;
//     app_inst->terminate = appTerminate;

//     app_inst->data = NULL;

//     return true;
// }

Application app = {
    .config =
        {.name = "Snuk Testapp", .x = 50, .y = 50, .width = 600, .height = 400},
    .data = NULL,
    .initialize = appInitialize,
    .update = appUpdate,
    .render = appRender,
    .terminate = appTerminate
};

b8 key_press_logger(u16 code, void *sender, void *listener,
                    EventContext context) {
    UNUSED(sender);
    UNUSED(listener);
    if (code != EVENT_CODE_KEY_PRESS) sError("Getting unregisterd events!");

    u32 sc = context.data.u32[0];
    u32 kc = context.data.u32[1];
    u32 mod = context.data.u32[2];

    sInfo("Pressed scancode = '%d', keycode = '%d', keymod = '%u'", sc, kc,
          mod);

    return false;
}

b8 key_release_logger(u16 code, void *sender, void *listener,
                      EventContext context) {
    UNUSED(sender);
    UNUSED(listener);
    if (code != EVENT_CODE_KEY_RELEASE) sError("Getting unregisterd events!");

    u32 sc = context.data.u32[0];
    u32 kc = context.data.u32[1];
    u32 mod = context.data.u32[2];

    sInfo("Released scancode = '%d', keycode = '%d', keymod = '%u'", sc, kc,
          mod);

    return false;
}

b8 button_press_logger(u16 code, void *sender, void *listener,
                       EventContext context) {
    UNUSED(sender);
    UNUSED(listener);
    if (code != EVENT_CODE_BUTTON_PRESS) sError("Getting unregisterd events!");

    u32 button = context.data.u32[0];
    u32 x = context.data.u32[1];
    u32 y = context.data.u32[2];
    u32 mod = context.data.u32[3];

    sInfo("Pressed '%d' at (%u, %u), mod = %u", button, x, y, mod);

    return false;
}

b8 button_release_logger(u16 code, void *sender, void *listener,
                         EventContext context) {
    UNUSED(sender);
    UNUSED(listener);
    if (code != EVENT_CODE_BUTTON_RELEASE)
        sError("Getting unregisterd events!");

    u32 button = context.data.u32[0];
    u32 x = context.data.u32[1];
    u32 y = context.data.u32[2];
    u32 mod = context.data.u32[3];

    sInfo("Released '%d' at (%u, %u), mod = %u", button, x, y, mod);

    return false;
}

b8 scroll_logger(u16 code, void *sender, void *listener, EventContext context) {
    UNUSED(sender);
    UNUSED(listener);
    if (code != EVENT_CODE_SCROLL) sError("Getting unregistered events!");

    u32 direction = context.data.u32[0];
    u32 delta = context.data.u32[1];
    u32 mod = context.data.u32[2];

    sInfo("Direction '%u', delta = '%u', mod = %u", direction, delta, mod);

    return false;
}
