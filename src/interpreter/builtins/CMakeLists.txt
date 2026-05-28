set(HEADERS
    builtin_int.h
    builtin_float.h
    builtin_bool.h
    builtin_str.h
    snuk_builtins.h
    builtins_common.h
)

set(SRCS
    builtin_int.c
    builtin_float.c
    builtin_bool.c
    builtin_str.c
)

target_sources(snuk PRIVATE ${HEADERS} ${SRCS})
