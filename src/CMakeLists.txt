add_library(snuk STATIC)
target_link_libraries(snuk PUBLIC snuk_configs)

set(PUBLIC_HEADERS
    defines.h
    logger.h
    memory.h
    io.h
    snuk_string.h
    lexer.h
    darray.h
)

set(HEADERS
)

set(SRCS
    logger.c
    memory.c
    io.c
    lexer.c
    darray.c
)

set(INCLUDE_BASE "${PROJECT_SOURCE_DIR}/include/snuk")

list(TRANSFORM PUBLIC_HEADERS PREPEND "${INCLUDE_BASE}/")

target_include_directories(snuk PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(snuk PRIVATE ${PUBLIC_HEADERS} ${HEADERS} ${SRCS})

add_subdirectory(parser)
add_subdirectory(interpreter)
