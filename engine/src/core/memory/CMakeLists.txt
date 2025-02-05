set(SRCS
    arenaalloc.c
    arenaalloc.h
    memory.c
    memory.h
)

target_sources("${TARGET_NAME}" PRIVATE ${SRCS})
