set(SRCS
    atomic.h
    atomic.c
)

if(MSVC)
    set(ASM_SRCS
        atomic.asm
    )

    list(APPEND SRCS ${ASM_SRCS})
endif()

target_sources(snuk-test PRIVATE ${SRCS})
