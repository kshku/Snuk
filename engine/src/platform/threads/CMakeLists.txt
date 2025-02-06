set(SUBDIRS
    pthread
    win32
)

foreach(directory IN LISTS SUBDIRS)
    add_subdirectory("${directory}")
endforeach()
