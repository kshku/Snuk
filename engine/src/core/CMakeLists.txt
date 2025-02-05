set(SRCS
    assertions.h
    engine.c
    engine.h
    event.c
    event.h
    logger.c
    logger.h
    sstring.c
    sstring.h
)

target_sources("${TARGET_NAME}" PRIVATE ${SRCS})

set(SUBDIRS memory sync)

foreach(directory IN LISTS SUBDIRS)
    add_subdirectory("${directory}")
endforeach()
