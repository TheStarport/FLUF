cmake_minimum_required(VERSION 3.23.5)

set(VER_MAJOR 1)
set(VER_MINOR 0)
set(VER_PATCH 0)
set(NAME "Patch Notes")
set(DESCRIPTION "This adds to the main menu a box containing information that is pulled from a web server on startup.")
set(CUR_PROJECT "patch_notes")

# Glob our source files and header files
begin_glob()

configure_file(${ROOT_DIR}/metadata.rc.in ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc @ONLY)
add_library(${CUR_PROJECT} SHARED ${SOURCE_FILES} ${ROOT_DIR}/PCH.cpp ${ROOT_DIR}/PCH.hpp ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc)

# Dependencies
target_include_directories(${CUR_PROJECT} PRIVATE
        ${FLUF_UI_PATH}/Include
        ${CURL_PATH}/include
        ${VENDOR_PATH}/imgui-markdown
)
add_dependencies(${CUR_PROJECT} FLUF FLUF.UI)
target_link_libraries(${CUR_PROJECT} PRIVATE
        FLUF
        FLUF.UI
        "${SDK_PATH}/lib/FLCoreCommon.lib"
        ${VENDOR_PATH}/libcurl.lib
)

target_precompile_headers(${CUR_PROJECT} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${ROOT_DIR}/PCH.hpp>")

set_general_targets(PROJECT_NAME ${CUR_PROJECT})
set_linux_msvc_includes(PROJECT_NAME ${CUR_PROJECT})
create_post_build(TARGET_NAME ${CUR_PROJECT})

reset_glob()
