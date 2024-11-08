cmake_minimum_required(VERSION 3.23.5)

set(VER_MAJOR 1)
set(VER_MINOR 0)
set(VER_PATCH 0)
set(NAME "Group Info")
set(DESCRIPTION "Displays a 'party window', showing the stats of your group members within the same system.")
set(GROUP_INFO_PATH ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

# Glob our source files and header files
begin_glob()

configure_file(${ROOT_DIR}/metadata.rc.in ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc @ONLY)
add_library(group_info SHARED ${SOURCE_FILES} ${ROOT_DIR}/PCH.cpp ${ROOT_DIR}/PCH.hpp ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc)

# Dependencies
target_include_directories(group_info PRIVATE ${RML_PATH}/Include)
target_include_directories(group_info PRIVATE ${FLUF_UI_PATH}/Include)
add_dependencies(group_info FLUF FLUF.UI)
target_link_libraries(group_info PRIVATE FLUF FLUF.UI RmlUi::Core)

# Link against FLCore
target_link_libraries(group_info PRIVATE "${SDK_PATH}/lib/FLCoreCommon.lib")

target_precompile_headers(group_info PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${ROOT_DIR}/PCH.hpp>")

set_general_targets(PROJECT_NAME group_info)
set_linux_msvc_includes(PROJECT_NAME group_info)
create_post_build(TARGET_NAME group_info)

reset_glob()
