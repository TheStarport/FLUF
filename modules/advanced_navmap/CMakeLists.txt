cmake_minimum_required(VERSION 3.23.5)

set(VER_MAJOR 1)
set(VER_MINOR 0)
set(VER_PATCH 0)
set(NAME "Advanced NavMap")
set(DESCRIPTION "Advanced Navigation Map for Freelancer.")
set(GROUP_INFO_PATH . PARENT_SCOPE)
set(CUR_PROJECT "advanced_navmap")

# Glob our source files and header files
begin_glob()

configure_file(${ROOT_DIR}/metadata.rc.in ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc @ONLY)
add_library(advanced_navmap SHARED ${SOURCE_FILES} ${ROOT_DIR}/PCH.cpp ${ROOT_DIR}/PCH.hpp ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc)

# Dependencies
target_include_directories(advanced_navmap PRIVATE ${RML_PATH}/Include)
target_include_directories(advanced_navmap PRIVATE ${FLUF_UI_PATH}/Include)
add_dependencies(advanced_navmap FLUF FLUF.UI)
target_link_libraries(advanced_navmap PRIVATE FLUF FLUF.UI)

# Link against FLCore
target_link_libraries(advanced_navmap PRIVATE "${SDK_PATH}/lib/FLCoreCommon.lib")
target_link_libraries(advanced_navmap PRIVATE "${SDK_PATH}/lib/FLCoreServer.lib")
target_link_libraries(advanced_navmap PRIVATE "${SDK_PATH}/lib/FLCoreDACom.lib")
target_link_libraries(advanced_navmap PRIVATE "${SDK_PATH}/lib/FLCoreDALib.lib")
target_link_libraries(advanced_navmap PRIVATE "${SDK_PATH}/lib/FLCoreFLServerEXE.lib")
target_link_libraries(advanced_navmap PRIVATE "${SDK_PATH}/lib/FLCoreRemoteClient.lib")



target_precompile_headers(advanced_navmap PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${ROOT_DIR}/PCH.hpp>")

set_general_targets(PROJECT_NAME advanced_navmap)
set_linux_msvc_includes(PROJECT_NAME advanced_navmap)
create_post_build(TARGET_NAME advanced_navmap)

reset_glob()