set(HEADERS
    defines.h
    logger.h
)

set(SRCS
    main.c
    logger.c
)

target_sources(snuk PRIVATE ${HEADERS} ${SRCS})
