set(VER_MAJOR 1)
set(VER_MINOR 0)
set(VER_PATCH 0)
set(NAME "Retold")
set(DESCRIPTION "A module to enable all the mechanics that are used by the Retold project")
set(CUR_PROJECT "retold")

# Glob our source files and header files
begin_glob()

configure_file(${ROOT_DIR}/metadata.rc.in ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc @ONLY)
add_library(${CUR_PROJECT} SHARED
        ${SOURCE_FILES}
        ${ROOT_DIR}/PCH.cpp
        ${ROOT_DIR}/PCH.hpp
        ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc
        ${VENDOR_PATH}/inih/ini.c
)

# Dependencies
target_include_directories(${CUR_PROJECT} PRIVATE
        ${INCLUDE_PATH}
        ${FLUF_UI_PATH}/Include
        ${VENDOR_PATH}/inih
)
add_dependencies(${CUR_PROJECT} FLUF FLUF.UI)
target_link_libraries(${CUR_PROJECT} PRIVATE
        FLUF
        FLUF.UI
)

# Link against FLCore
target_link_libraries(${CUR_PROJECT} PRIVATE
        "${SDK_PATH}/lib/FLCoreCommon.lib"
        "${SDK_PATH}/lib/FLCoreServer.lib"
)

target_precompile_headers(${CUR_PROJECT} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${ROOT_DIR}/PCH.hpp>")

set_general_targets(PROJECT_NAME ${CUR_PROJECT})
set_linux_msvc_includes(PROJECT_NAME ${CUR_PROJECT})
create_post_build(TARGET_NAME ${CUR_PROJECT})

reset_glob()
