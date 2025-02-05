set(SUBDIRS xcb xlib)

foreach(directory IN LISTS SUBDIRS)
    add_subdirectory("${directory}")
endforeach()
