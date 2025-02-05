set(SRCS
    application_types.h
    defines.h
    entry.h
)

target_sources("${TARGET_NAME}" PRIVATE ${SRCS})

set(SUBDIRS core ds input platform)

foreach(directory IN LISTS SUBDIRS)
    add_subdirectory("${directory}")
endforeach()
