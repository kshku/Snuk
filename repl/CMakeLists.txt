add_executable(snuk_repl)
set_target_properties(snuk_repl PROPERTIES OUTPUT_NAME "snuk")

target_link_libraries(snuk_repl PRIVATE snuk)

set(HEADERS
    runtime.h
)

set(SRCS
    runtime.c
    main.c
)

target_sources(snuk_repl PRIVATE ${HEADERS} ${SRCS})
