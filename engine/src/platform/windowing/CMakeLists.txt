set(SRCS
    linux_input_helper.c
    linux_input_helper.h
)

target_sources("${TARGET_NAME}" PRIVATE ${SRCS})

set(SUBDIRS wayland win32 x11)

foreach(directory IN LISTS SUBDIRS)
    add_subdirectory("${directory}")
endforeach()
