set(SRCS
    defines.h
)

target_sources(snuk-test PRIVATE ${SRCS})

add_subdirectory(amd64)
