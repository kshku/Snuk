set(PUBLIC_HEADERS
    snuk_item.h
    snuk_expr.h
    snuk_type.h
    parser.h
    snuk_var.h
    parser_common.h
)

set(HEADERS
)

set(SRCS
    parser.c
    snuk_item.c
    snuk_expr.c
    snuk_type.c
    snuk_var.c
)

set(INCLUDE_BASE "${PROJECT_SOURCE_DIR}/include/snuk/parser")
list(TRANSFORM PUBLIC_HEADERS PREPEND "${INCLUDE_BASE}/")

target_sources(snuk PRIVATE ${PUBLIC_HEADERS} ${HEADERS} ${SRCS})
