set(SRCS
    mutex.c
    mutex.h
)

target_sources("${TARGET_NAME}" PRIVATE ${SRCS})
