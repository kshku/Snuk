set(TARGET_NAME "snuk")

add_library("${TARGET_NAME}" SHARED)

target_include_directories("${TARGET_NAME}" PUBLIC src)

target_compile_definitions("${TARGET_NAME}" PRIVATE S_EXPORTS)

target_link_libraries("${TARGET_NAME}" snuk_flags)

add_subdirectory(src)
