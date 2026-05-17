set(HEADERS
    parser_common.h
    snuk_item.h
    snuk_expr.h
    snuk_type.h
    snuk_param.h
    parser.h
)

set(SRCS
    parser.c
    snuk_item.c
    snuk_expr.c
    snuk_type.c
    snuk_param.c
)

target_sources(snuk PRIVATE ${HEADERS} ${SRCS})
