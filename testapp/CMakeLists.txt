set(TARGET_NAME "testapp")

add_executable("${TARGET_NAME}")

target_link_libraries("${TARGET_NAME}" PRIVATE snuk snuk_flags)

add_subdirectory(src)

add_custom_command(
    COMMENT "Copying the snuk $<TARGET_FILE:snuk> to $<TARGET_FILE_DIR:${TARGET_NAME}> build directory"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:snuk>" "$<TARGET_FILE_DIR:${TARGET_NAME}>"
    TARGET "${TARGET_NAME}" POST_BUILD
    VERBATIM
)
