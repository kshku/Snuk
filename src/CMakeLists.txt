set(HEADERS
    defines.h
    logger.h
    memory.h
    io.h
    string.h
    lexer.h
    parser.h
    parser_helper.h
)

set(SRCS
    main.c
    logger.c
    memory.c
    io.c
    lexer.c
    parser.c
)

target_sources(snuk PRIVATE ${HEADERS} ${SRCS})
