cmake_minimum_required(VERSION 3.15...3.31)

project("Snuk"
    VERSION 0.0
    DESCRIPTION "This is my playground"
)

add_library(snuk_flags INTERFACE)

target_compile_features(snuk_flags INTERFACE c_std_17)

target_compile_definitions(snuk_flags INTERFACE S_DEBUG)

set(SUBDIRS engine testapp tests)

foreach(directory IN LISTS SUBDIRS)
    add_subdirectory("${directory}")
endforeach()
