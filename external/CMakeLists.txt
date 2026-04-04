add_subdirectory(SnLogger)
add_subdirectory(SnMemory)
add_subdirectory(SnFile)

target_link_libraries(snuk PRIVATE snlogger snmemory snfile)
