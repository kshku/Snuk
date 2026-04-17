set(HEADERS
    defines.h
    logger.h
    memory.h
    io.h
    snuk_string.h
    lexer.h
    parser.h
    parser_helper.h
    darray.h
)

set(SRCS
    main.c
    logger.c
    memory.c
    io.c
    lexer.c
    parser.c
    darray.c
)

target_sources(snuk PRIVATE ${HEADERS} ${SRCS})
