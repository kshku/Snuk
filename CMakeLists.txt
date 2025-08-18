cmake_minimum_required(VERSION 3.15...3.31)

project("Snuk")

if(MSVC)
    enable_language(ASM_MASM)
endif()

add_executable(snuk-test)

set(SRCS
    test.c
)

target_sources(snuk-test PRIVATE ${SRCS})

add_subdirectory(src)
