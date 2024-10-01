begin_glob()

# Conditionally add debug flags
add_compile_options("$<$<CONFIG:DEBUG>:-D_DEBUG>")

add_library(FLUF.UI SHARED ${SOURCE_FILES} ${ROOT_DIR}/PCH.cpp ${ROOT_DIR}/PCH.hpp)

set_general_targets(PROJECT_NAME FLUF.UI)
set_target_properties(FLUF.UI PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(FLUF.UI PRIVATE ${RML_PATH}/Include)
target_include_directories(FLUF.UI PRIVATE ${CURL_PATH}/include)

target_link_libraries(FLUF.UI PRIVATE "${SDK_PATH}/lib/FLCoreCommon.lib")

target_compile_definitions(FLUF.UI PRIVATE FLUF_UI PRIVATE FLCore)

target_precompile_headers(FLUF.UI PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${ROOT_DIR}/PCH.hpp>")

# DirectX9 SDK
set(DXSDK "${PROJECT_SOURCE_DIR}/vendor/DXSDK")
target_include_directories(FLUF.UI PRIVATE ${DXSDK}/Include)
target_link_libraries(FLUF.UI PRIVATE ${DXSDK}/Lib/x86/d3dx9.lib)

# LibCurl
target_include_directories(FLUF.UI PRIVATE ${CURL_PATH}/include)
#target_link_libraries(FLUF.UI PRIVATE ${VENDOR_PATH}/libcurl.lib)

# Selectively skip PCH for certain files
set_source_files_properties(
        ${REFLECT_PATH}/src/yyjson.c
        ${REFLECT_PATH}/src/reflectcpp.cpp
        PROPERTIES SKIP_PRECOMPILED_HEADERS ON
)

# RmlUi
add_dependencies(FLUF.UI RmlUi::RmlUi RmlUi::Debugger RmlUi::Lua)
target_link_libraries(FLUF.UI PRIVATE RmlUi::Core RmlUi::Debugger RmlUi::Lua)

set_linux_msvc_includes(PROJECT_NAME FLUF.UI)

reset_glob()