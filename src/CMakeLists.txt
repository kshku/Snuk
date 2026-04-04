set(HEADERS
    defines.h
    logger.h
    memory.h
    io.h
)

set(SRCS
    main.c
    logger.c
    memory.c
    io.c
)

target_sources(snuk PRIVATE ${HEADERS} ${SRCS})
