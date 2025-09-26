cmake_minimum_required(VERSION 3.23.5)

set(VER_MAJOR 1)
set(VER_MINOR 0)
set(VER_PATCH 0)
set(NAME "QOL Patcher")
set(DESCRIPTION "Change various hardcoded settings via a nice friendly UI")
set(CUR_PROJ qol_patcher)

# Glob our source files and header files
begin_glob()

configure_file(${ROOT_DIR}/metadata.rc.in ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc @ONLY)
add_library(${CUR_PROJ} SHARED
        ${SOURCE_FILES}
        ${ROOT_DIR}/PCH.cpp
        ${ROOT_DIR}/PCH.hpp
        ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc
)

# Dependencies
target_include_directories(${CUR_PROJ} PRIVATE
        ${INCLUDE_PATH}
        ${FLUF_UI_PATH}/Include
)

add_dependencies(${CUR_PROJ}
        FLUF
        FLUF.UI
)

target_link_libraries(${CUR_PROJ} PRIVATE
        FLUF
        FLUF.UI
)

# Link against FLCore
target_link_libraries(${CUR_PROJ} PRIVATE "${SDK_PATH}/lib/FLCoreCommon.lib")

target_precompile_headers(${CUR_PROJ} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${ROOT_DIR}/PCH.hpp>")

set_general_targets(PROJECT_NAME ${CUR_PROJ})
set_linux_msvc_includes(PROJECT_NAME ${CUR_PROJ})
create_post_build(TARGET_NAME ${CUR_PROJ})

reset_glob()
