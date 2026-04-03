add_subdirectory(SnLogger)
add_subdirectory(SnMemory)

target_link_libraries(snuk PRIVATE snlogger snmemory)
