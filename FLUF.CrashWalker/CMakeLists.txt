begin_glob()

set(VER_MAJOR 1)
set(VER_MINOR 0)
set(VER_PATCH 0)
set(NAME "FLUF Crash Walker")
set(DESCRIPTION "FLUF Crash Walker is a utility for detecting and preventing crashes, or providing information on why.")
set(FLUF_UI_PATH ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

configure_file(${ROOT_DIR}/metadata.rc.in ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc @ONLY)
add_library(FLUF.CrashWalker SHARED
        ${SOURCE_FILES}
        ${ROOT_DIR}/PCH.cpp
        ${ROOT_DIR}/PCH.hpp
        ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc
        ${VENDOR_PATH}/stack_walker/StackWalker.cpp
)

target_include_directories(FLUF.CrashWalker PRIVATE
        ${VENDOR_PATH}/stack_walker
        ${CURL_PATH}/include
)

target_compile_definitions(FLUF.CrashWalker PRIVATE FLUF_UI FLCore)
target_link_libraries(FLUF.CrashWalker PRIVATE
        "Dbghelp.lib"
        "${SDK_PATH}/lib/FLCoreCommon.lib"
        FLUF
        ${VENDOR_PATH}/libcurl.lib
)
add_dependencies(FLUF.CrashWalker FLUF)


# General
set_general_targets(PROJECT_NAME FLUF.CrashWalker)
set_linux_msvc_includes(PROJECT_NAME FLUF.CrashWalker)
create_post_build(TARGET_NAME FLUF.CrashWalker)

reset_glob()