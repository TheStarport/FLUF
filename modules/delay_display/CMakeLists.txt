cmake_minimum_required(VERSION 3.23.5)

set(VER_MAJOR 1)
set(VER_MINOR 0)
set(VER_PATCH 0)
set(NAME "Delay Display")
set(DESCRIPTION "Modify the ammo display to include cooldown for slow-firing weapons. Originally made by M0tah, reverse engineered by Aingar")
set(DELAY_DISPLAY_PATH ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

# Glob our source files and header files
begin_glob()

configure_file(${ROOT_DIR}/metadata.rc.in ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc @ONLY)
add_library(delay_display SHARED ${SOURCE_FILES} ${ROOT_DIR}/PCH.cpp ${ROOT_DIR}/PCH.hpp ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc)

# Dependencies
target_include_directories(delay_display PRIVATE ${FLUF_UI_PATH}/Include)
add_dependencies(delay_display FLUF FLUF.UI)
target_link_libraries(delay_display PRIVATE FLUF FLUF.UI)

# Link against FLCore
target_link_libraries(delay_display PRIVATE "${SDK_PATH}/lib/FLCoreCommon.lib")

target_precompile_headers(delay_display PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${ROOT_DIR}/PCH.hpp>")

set_general_targets(PROJECT_NAME delay_display)
set_linux_msvc_includes(PROJECT_NAME delay_display)
create_post_build(TARGET_NAME delay_display)

reset_glob()
