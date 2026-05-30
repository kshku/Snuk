set(PUBLIC_HEADERS
    snuk_builtins.h
)
set(HEADERS
)

set(SRCS
    builtin_int.c
    builtin_float.c
    builtin_bool.c
    builtin_str.c
    builtin_null.c
)

set(INCLUDE_BASE "${PROJECT_SOURCE_DIR}/include/snuk/interpreter/builtins")
list(TRANSFORM PUBLIC_HEADERS PREPEND "${INCLUDE_BASE}/")

target_sources(snuk PRIVATE ${PUBLIC_HEADERS} ${HEADERS} ${SRCS})
