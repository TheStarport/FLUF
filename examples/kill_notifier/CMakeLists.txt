cmake_minimum_required(VERSION 3.23.5)

set(VER_MAJOR 1)
set(VER_MINOR 0)
set(VER_PATCH 0)
set(NAME "Kill Notifier")
set(DESCRIPTION "Server Plugin, sends notifications to FLUF Clients about who killed who.")
set(SHIP_DEALER_ENHANCEMENTS_PATH ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

# Glob our source files and header files
begin_glob()

configure_file(${ROOT_DIR}/metadata.rc.in ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc @ONLY)
add_library(kill_notifier SHARED ${SOURCE_FILES} ${ROOT_DIR}/PCH.cpp ${ROOT_DIR}/PCH.hpp ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc)

# Dependencies
target_include_directories(kill_notifier PRIVATE ${RML_PATH}/Include)
target_include_directories(kill_notifier PRIVATE ${FLUF_UI_PATH}/Include)
add_dependencies(kill_notifier FLUF FLUF.UI)
target_link_libraries(kill_notifier PRIVATE FLUF FLUF.UI)

# Link against FLCore
target_link_libraries(kill_notifier PRIVATE "${SDK_PATH}/lib/FLCoreCommon.lib")

target_precompile_headers(kill_notifier PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${ROOT_DIR}/PCH.hpp>")

set_general_targets(PROJECT_NAME kill_notifier)
set_linux_msvc_includes(PROJECT_NAME kill_notifier)
create_post_build(TARGET_NAME kill_notifier)

reset_glob()
