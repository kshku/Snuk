set(SUBDIRS linux windows)

foreach(directory IN LISTS SUBDIRS)
    add_subdirectory("${directory}")
endforeach()
