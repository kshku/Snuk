set(SRCS
    main.c
    test_manager.c
    test_manager.h
)

target_sources("${TARGET_NAME}" PRIVATE ${SRCS})

add_subdirectory(tests)
