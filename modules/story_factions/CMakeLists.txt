cmake_minimum_required(VERSION 3.23.5)

set(VER_MAJOR 1)
set(VER_MINOR 0)
set(VER_PATCH 0)
set(NAME "Story Factions")
set(DESCRIPTION "Defines story factions from a configuration file")

# Glob our source files and header files
begin_glob()

configure_file(${ROOT_DIR}/metadata.rc.in ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc @ONLY)
add_library(story_factions SHARED ${SOURCE_FILES} ${ROOT_DIR}/PCH.cpp ${ROOT_DIR}/PCH.hpp ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc)

# Dependencies
target_include_directories(story_factions PRIVATE ${RML_PATH}/Include)
add_dependencies(story_factions FLUF)
target_link_libraries(story_factions PRIVATE FLUF)

# Link against FLCore
target_link_libraries(story_factions PRIVATE "${SDK_PATH}/lib/FLCoreCommon.lib")

target_precompile_headers(story_factions PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${ROOT_DIR}/PCH.hpp>")

set_general_targets(PROJECT_NAME story_factions)
set_linux_msvc_includes(PROJECT_NAME story_factions)
create_post_build(TARGET_NAME story_factions)

reset_glob()