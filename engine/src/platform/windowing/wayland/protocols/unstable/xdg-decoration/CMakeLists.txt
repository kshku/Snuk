set(XDG_DECORATION_C "${CMAKE_CURRENT_SOURCE_DIR}/xdg-decoration-unstable-v1.c")

set(XDG_DECORATOR_H "${CMAKE_CURRENT_SOURCE_DIR}/xdg-decoration-unstable-v1.h")

set(SRCS ${XDG_DECORATION_C} ${XDG_DECORATION_H})

set(XMLSRC "${CMAKE_CURRENT_SOURCE_DIR}/xdg-decoration-unstable-v1.xml")

add_custom_command(
    OUTPUT ${SRCS}
    COMMAND wayland-scanner private-code "${XMLSRC}" ${XDG_DECORATION_C}
    COMMAND wayland-scanner client-header "${XMLSRC}" ${XDG_DECORATOR_H}
    MAIN_DEPENDENCY "${XMLSRC}"
    COMMENT "Generating source files from ${XMLSRC} protocol file"
    VERBATIM
)

add_custom_target(generate_xdg_decorator DEPENDS ${SRCS})

add_dependencies("${TARGET_NAME}" generate_xdg_decorator)

target_sources("${TARGET_NAME}" PRIVATE ${SRCS})
