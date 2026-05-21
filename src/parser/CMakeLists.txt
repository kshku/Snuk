set(HEADERS
    parser_common.h
    snuk_item.h
    snuk_expr.h
    snuk_type.h
    parser.h
    snuk_var.h
)

set(SRCS
    parser.c
    snuk_item.c
    snuk_expr.c
    snuk_type.c
    snuk_var.c
)

target_sources(snuk PRIVATE ${HEADERS} ${SRCS})
