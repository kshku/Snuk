set(SRCS
    input_helper.c
    input_helper.h
    window.c
)

target_sources("${TARGET_NAME}" PRIVATE ${SRCS})
