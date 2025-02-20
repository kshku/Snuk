set(TARGET_NAME "snuk")

add_library("${TARGET_NAME}" SHARED)

target_include_directories("${TARGET_NAME}" PUBLIC src)

target_compile_definitions("${TARGET_NAME}" PRIVATE S_EXPORTS)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_options("${TARGET_NAME}" PRIVATE -fvisibility=hidden)
endif()

target_link_libraries("${TARGET_NAME}" snuk_flags)

add_subdirectory(src)
