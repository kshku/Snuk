set(PUBLIC_HEADERS
    interpreter.h
    snuk_signal.h
    snuk_value.h
    snuk_scope.h
    snuk_env.h
)

set(HEADERS
)

set(SRCS
    interpreter.c
    snuk_value.c
)

set(INCLUDE_BASE "${PROJECT_SOURCE_DIR}/include/snuk/interpreter")

list(TRANSFORM PUBLIC_HEADERS PREPEND "${INCLUDE_BASE}/")

target_sources(snuk PRIVATE ${PUBLIC_HEADERS} ${HEADERS} ${SRCS})

add_subdirectory(builtins)
