add_executable(concat_test string_concat.c ../src/logger.c ../src/memory.c) 

target_include_directories(concat_test PRIVATE "${CMAKE_SOURCE_DIR}/src")

target_link_libraries(concat_test PRIVATE snlogger snmemory)
