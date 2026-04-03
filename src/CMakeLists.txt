set(HEADERS
    defines.h
    logger.h
    memory.h
)

set(SRCS
    main.c
    logger.c
    memory.c
)

target_sources(snuk PRIVATE ${HEADERS} ${SRCS})
