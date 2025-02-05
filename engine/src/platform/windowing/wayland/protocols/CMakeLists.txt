if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_program(wayland_scanner_exists wayland-scanner)

    if(NOT wayland_scanner_exists)
        # TODO: Only exit if you are builing for wayland
        message(WARNING "wayland-scanner is not found! If you are not building for wayland ignore this")
    endif()

    add_library(protocols STATIC)
    set(SUBDIRS stable unstable)

    foreach(directory IN LISTS SUBDIRS)
        add_subdirectory("${directory}")
    endforeach()
endif()
