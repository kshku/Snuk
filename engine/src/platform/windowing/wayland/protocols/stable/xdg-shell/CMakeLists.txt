set(XDG_SHELL_C "${CMAKE_CURRENT_SOURCE_DIR}/xdg-shell.c")

set(XDG_SHELL_H "${CMAKE_CURRENT_SOURCE_DIR}/xdg-shell.h")

set(SRCS ${XDG_SHELL_C} ${XDG_SHELL_H})

set(XMLSRC "${CMAKE_CURRENT_SOURCE_DIR}/xdg-shell.xml")

add_custom_command(
    OUTPUT ${SRCS}
    COMMAND wayland-scanner private-code "${XMLSRC}" "${XDG_SHELL_C}"
    COMMAND wayland-scanner client-header "${XMLSRC}" "${XDG_SHELL_H}"
    MAIN_DEPENDENCY "${XMLSRC}"
    COMMENT "Generating source files from ${XMLSRC} protocol file"
    VERBATIM
)

add_custom_target(generate_xdg_shell DEPENDS ${SRCS})

add_dependencies("${TARGET_NAME}" generate_xdg_shell)

target_sources("${TARGET_NAME}" PRIVATE ${SRCS})
