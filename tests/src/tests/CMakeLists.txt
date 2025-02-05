set(SRCS
    atomic.c
    atomic.h
    darray.c
    darray.h
    log.c
    log.h
    memory.c
    memory.h
    sstring.c
    sstring.h
)

target_sources("${TARGET_NAME}" PRIVATE ${SRCS})
