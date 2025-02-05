set(SRCS
    atomic.h
    log.h
    memory.h
    threads.h
    window.h
)

target_sources("${TARGET_NAME}" PRIVATE ${SRCS})

set(SUBDIRS arch os threads windowing)

foreach(directory IN LISTS SUBDIRS)
    add_subdirectory("${directory}")
endforeach()
