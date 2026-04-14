cmake_minimum_required(VERSION 3.15...3.31)

project(Snuk VERSION 0.0.0 DESCRIPTION "Snuk" LANGUAGES C)

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)

add_library(snuk_configs INTERFACE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(snuk_configs INTERFACE SNUK_DEBUG)
endif()

set(gcc_clang_comp "$<COMPILE_LANG_AND_ID:C,Clang,GNU>")
set(msvc_comp "$<COMPILE_LANG_AND_ID:C,MSVC>")

target_compile_options(snuk_configs INTERFACE "$<${gcc_clang_comp}:-Wall;-Wextra;-Wformat;-Wuninitialized;-Wpedantic>")
target_compile_options(snuk_configs INTERFACE "$<${gcc_clang_comp}:-fvisibility=hidden>")

target_compile_definitions(snuk_configs INTERFACE
    SNUK_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    SNUK_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    SNUK_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

add_executable(snuk)
target_link_libraries(snuk PRIVATE snuk_configs)

add_subdirectory(external)

add_subdirectory(docs)

add_subdirectory(src)
