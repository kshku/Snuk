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

target_compile_options(snuk_configs INTERFACE "$<${gcc_clang_comp}:-Wall;-Wextra;-Wformat;-Wuninitialized;-Wunused;-Wpedantic>")
target_compile_options(snuk_configs INTERFACE "$<${gcc_clang_comp}:-fvisibility=hidden>")

target_include_directories(snuk_configs INTERFACE ${PROJECT_SOURCE_DIR}/include)

target_compile_definitions(snuk_configs INTERFACE
    SNUK_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    SNUK_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    SNUK_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

option(SNUK_BUILD_SHARED "Build shared library" OFF)

include(FetchContent)

FetchContent_Declare(
    snlogger
    GIT_REPOSITORY https://github.com/kshku/SnLogger.git
    GIT_TAG v0.2.0
)
FetchContent_MakeAvailable(snlogger)

FetchContent_Declare(
    snfile
    GIT_REPOSITORY https://github.com/kshku/SnFile.git
    GIT_TAG v0.1.0
)
FetchContent_MakeAvailable(snfile)

FetchContent_Declare(
    snmemory
    GIT_REPOSITORY https://github.com/kshku/SnMemory.git
    GIT_TAG v0.2.0
)

FetchContent_Declare(
    sncontainer
    GIT_REPOSITORY https://github.com/kshku/SnContainer.git
    GIT_TAG v0.1.0
)
FetchContent_MakeAvailable(sncontainer)

target_link_libraries(snuk_configs INTERFACE snlogger snfile snmemory sncontainer)

add_subdirectory(docs)
add_subdirectory(src)
add_subdirectory(repl)
add_subdirectory(tests)
