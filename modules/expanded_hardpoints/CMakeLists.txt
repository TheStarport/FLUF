cmake_minimum_required(VERSION 3.23.5)

set(VER_MAJOR 1)
set(VER_MINOR 0)
set(VER_PATCH 0)
set(NAME "Expanded Hardpoints")
set(DESCRIPTION "Allows definition of unlimited amounts of equipment subclasses")

# Glob our source files and header files
begin_glob()

configure_file(${ROOT_DIR}/metadata.rc.in ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc @ONLY)
add_library(expanded_hardpoints SHARED ${SOURCE_FILES} ${ROOT_DIR}/PCH.cpp ${ROOT_DIR}/PCH.hpp ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc "Source/GameDataReading.cpp")

# Dependencies
target_include_directories(expanded_hardpoints PRIVATE ${RML_PATH}/Include)
add_dependencies(expanded_hardpoints FLUF)
target_link_libraries(expanded_hardpoints PRIVATE FLUF)

# Link against FLCore
target_link_libraries(expanded_hardpoints PRIVATE "${SDK_PATH}/lib/FLCoreCommon.lib")

target_precompile_headers(expanded_hardpoints PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${ROOT_DIR}/PCH.hpp>")

set_general_targets(PROJECT_NAME expanded_hardpoints)
set_linux_msvc_includes(PROJECT_NAME expanded_hardpoints)
create_post_build(TARGET_NAME expanded_hardpoints)

reset_glob()