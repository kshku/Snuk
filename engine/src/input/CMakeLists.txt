set(SRCS
    input.c
    input.h
)

target_sources("${TARGET_NAME}" PRIVATE ${SRCS})

set(SUBDIRS keyboard mouse)

foreach(directory IN LISTS SUBDIRS)
    add_subdirectory("${directory}")
endforeach()
