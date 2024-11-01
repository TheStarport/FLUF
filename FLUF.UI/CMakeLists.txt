begin_glob()

set(VER_MAJOR 1)
set(VER_MINOR 0)
set(VER_PATCH 0)
set(NAME "FLUF UI")
set(DESCRIPTION "FLUF UI is a framework designed to making UI extensions for Freelancer (2003) possible")
set(FLUF_UI_PATH ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

configure_file(${ROOT_DIR}/metadata.rc.in ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc @ONLY)
add_library(FLUF.UI SHARED ${SOURCE_FILES} ${ROOT_DIR}/PCH.cpp ${ROOT_DIR}/PCH.hpp ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc)

target_include_directories(FLUF.UI PRIVATE ${RML_PATH}/Include)
target_include_directories(FLUF.UI PRIVATE ${CURL_PATH}/include)

target_link_libraries(FLUF.UI PRIVATE "${SDK_PATH}/lib/FLCoreCommon.lib")

target_compile_definitions(FLUF.UI PRIVATE FLUF_UI FLCore)

# DirectX9 SDK
set(DXSDK "${PROJECT_SOURCE_DIR}/vendor/DXSDK")
target_include_directories(FLUF.UI PRIVATE ${DXSDK}/Include)
target_link_libraries(FLUF.UI PRIVATE ${DXSDK}/Lib/x86/d3dx9.lib)

# LibCurl
target_include_directories(FLUF.UI PRIVATE ${CURL_PATH}/include)
#target_link_libraries(FLUF.UI PRIVATE ${VENDOR_PATH}/libcurl.lib)

# RmlUi
add_dependencies(FLUF.UI FLUF RmlUi::RmlUi RmlUi::Debugger RmlUi::Lua)
target_link_libraries(FLUF.UI PRIVATE FLUF RmlUi::Core RmlUi::Debugger RmlUi::Lua)

# General
set_general_targets(PROJECT_NAME FLUF.UI)
set_linux_msvc_includes(PROJECT_NAME FLUF.UI)
create_post_build(TARGET_NAME FLUF.UI)

reset_glob()