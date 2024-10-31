cmake_minimum_required(VERSION 3.23.5)

set(VER_MAJOR 1)
set(VER_MINOR 0)
set(VER_PATCH 0)
set(NAME "FLUF")
set(DESCRIPTION "FLUF is a framework designed to making writing extensions for Freelancer (2003) easier")

# Glob our source files and header files
begin_glob()

configure_file(${ROOT_DIR}/metadata.rc.in ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc @ONLY)
add_library(FLUF SHARED ${SOURCE_FILES} ${ROOT_DIR}/PCH.cpp ${ROOT_DIR}/PCH.hpp ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc)

# Link against FLCore
target_link_libraries(FLUF PRIVATE "${SDK_PATH}/lib/FLCoreCommon.lib")

target_compile_definitions(FLUF PRIVATE FLUF FLCore)

target_precompile_headers(FLUF PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${ROOT_DIR}/PCH.hpp>")

set_general_targets(PROJECT_NAME FLUF)
set_linux_msvc_includes(PROJECT_NAME FLUF)
create_post_build(TARGET_NAME FLUF)

reset_glob()