set(HEADERS
    defines.h
    logger.h
    memory.h
    io.h
    snuk_string.h
    lexer.h
    darray.h
    runtime.h
)

set(SRCS
    main.c
    logger.c
    memory.c
    io.c
    lexer.c
    darray.c
    runtime.c
)

target_include_directories(snuk PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(snuk PRIVATE ${HEADERS} ${SRCS})

add_subdirectory(parser)
add_subdirectory(interpreter)
