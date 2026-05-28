function(add_snuk_test name source)
    add_executable(${name}
        ${source}
        ${PROJECT_SOURCE_DIR}/src/logger.c
        ${PROJECT_SOURCE_DIR}/src/memory.c
    )
    add_dependencies(run_tests ${name})
    target_include_directories(${name} PRIVATE ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(${name} PRIVATE snlogger snmemory snfile)
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES LABELS "unit")
endfunction()

file(GLOB files CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
foreach(file IN LISTS files)
    cmake_path(GET file STEM file_name_we)
    add_snuk_test(test_${file_name_we} ${file})
endforeach()
