enable_testing()

add_custom_target(run_tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS snuk
    COMMENT "Run all the tests"
    COMMAND ctest --test-dir ${CMAKE_CURRENT_BINARY_DIR} -L unit --output-on-failure
    COMMAND ctest --test-dir ${CMAKE_CURRENT_BINARY_DIR} -L snuk_files --output-on-failure
)

add_subdirectory(unit)

function(run_snuk_file name source)
    add_test(NAME ${name} COMMAND $<TARGET_FILE:snuk> ${source})
    set_tests_properties(${name} PROPERTIES LABELS "snuk_files")
endfunction()

file(GLOB files CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.snuk")
foreach(file IN LISTS files)
    cmake_path(GET file STEM file_name_we)
    run_snuk_file(test_${file_name_we} ${file})
endforeach()

