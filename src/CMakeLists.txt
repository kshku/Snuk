set(HEADERS
    defines.h
    logger.h
    memory.h
    io.h
    string.h
    lexer.h
)

set(SRCS
    main.c
    logger.c
    memory.c
    io.c
    lexer.c
)

target_sources(snuk PRIVATE ${HEADERS} ${SRCS})
