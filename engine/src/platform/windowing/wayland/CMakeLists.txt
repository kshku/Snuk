set(SRCS
    input_helper.c
    input_helper.h
    window.c
)

target_sources("${TARGET_NAME}" PRIVATE ${SRCS})

add_subdirectory(protocols)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries("${TARGET_NAME}" protocols)
endif()
