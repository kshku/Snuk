set(HEADERS
    interpreter.h
    snuk_signal.h
    snuk_value.h
    snuk_scope.h
    snuk_env.h
)

set(SRCS
    interpreter.c
    snuk_value.c
)

target_sources(snuk PRIVATE ${HEADERS} ${SRCS})
