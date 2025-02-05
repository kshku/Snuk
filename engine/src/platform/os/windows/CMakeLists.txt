set(SRCS
    log.c
    memory.c
)

target_sources("${TARGET_NAME}" PRIVATE ${SRCS})
