set(SRCS
    keyboard.c
    keyboard.h
    keycode.h
    scancode.h
)

target_sources("${TARGET_NAME}" PRIVATE ${SRCS})
