cmake_minimum_required(VERSION 3.23.5)

set(VER_MAJOR 1)
set(VER_MINOR 0)
set(VER_PATCH 0)
set(NAME "Screen Control")
set(DESCRIPTION "Enables the game continuing to run in the background, as well as toggling between fullscreen and windowed modes")
set(DELAY_DISPLAY_PATH ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

# Glob our source files and header files
begin_glob()

configure_file(${ROOT_DIR}/metadata.rc.in ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc @ONLY)
add_library(screen_control SHARED ${SOURCE_FILES} ${ROOT_DIR}/PCH.cpp ${ROOT_DIR}/PCH.hpp ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc)

# Dependencies
target_include_directories(screen_control PRIVATE
        ${INCLUDE_PATH}
        ${FLUF_UI_PATH}/Include
)
add_dependencies(screen_control FLUF FLUF.UI)
target_link_libraries(screen_control PRIVATE FLUF FLUF.UI)

# Link against FLCore
target_link_libraries(screen_control PRIVATE "${SDK_PATH}/lib/FLCoreCommon.lib")

target_precompile_headers(screen_control PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${ROOT_DIR}/PCH.hpp>")

set_general_targets(PROJECT_NAME screen_control)
set_linux_msvc_includes(PROJECT_NAME screen_control)
create_post_build(TARGET_NAME screen_control)

reset_glob()
