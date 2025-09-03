begin_glob()

set(VER_MAJOR 1)
set(VER_MINOR 0)
set(VER_PATCH 0)
set(NAME "FLUF UI")
set(DESCRIPTION "FLUF UI is a framework designed to making UI extensions for Freelancer (2003) possible")
set(FLUF_UI_PATH ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

# ImGuiColorTextEdit dependency
set(IMGUI_COLOR_TEXT_EDIT_DIR "${VENDOR_PATH}/ImGuiColorTextEdit")
add_subdirectory(${IMGUI_COLOR_TEXT_EDIT_DIR}/vendor/regex ${CMAKE_CURRENT_BINARY_DIR}/regex EXCLUDE_FROM_ALL)
set(IMGUI_COLOR_TEXT_EDIT_SOURCE_FILES
        ${IMGUI_COLOR_TEXT_EDIT_DIR}/TextEditor.cpp
        ${IMGUI_COLOR_TEXT_EDIT_DIR}/LanguageDefinitions.cpp
)

set(IMGUI_TEXT_PATH ${VENDOR_PATH}/ImGuiTextUtils)

configure_file(${ROOT_DIR}/metadata.rc.in ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc @ONLY)
add_library(FLUF.UI SHARED
        ${SOURCE_FILES}
        ${ROOT_DIR}/PCH.cpp
        ${ROOT_DIR}/PCH.hpp
        ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc
        ${IMGUI_PATH}/imgui.cpp
        ${IMGUI_PATH}/imgui_demo.cpp
        ${IMGUI_PATH}/imgui_draw.cpp
        ${IMGUI_PATH}/imgui_tables.cpp
        ${IMGUI_PATH}/imgui_widgets.cpp
        ${IMGUI_PATH}/misc/cpp/imgui_stdlib.cpp
        ${IMGUI_PATH}/misc/freetype/imgui_freetype.cpp
        ${IMGUI_PATH}/backends/imgui_impl_win32.cpp
        ${IMGUI_PATH}/backends/imgui_impl_dx9.cpp
        ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp
        ${IMGUI_COLOR_TEXT_EDIT_SOURCE_FILES}
        ${IMGUI_TEXT_PATH}/UImGuiTextUtils.cpp
        ${IMGUI_TEXT_PATH}/UImGuiTextUtilsTextMarkdown.cpp
        ${IMGUI_TEXT_PATH}/UImGuiTextUtilsTypefaces.cpp
)

target_include_directories(FLUF.UI PRIVATE
        ${FREETYPE_INCLUDE_DIRS}
        ${IMGUI_PATH}/backends
        ${IMGUI_COLOR_TEXT_EDIT_DIR}
        ${IMGUI_COLOR_TEXT_EDIT_DIR}/vendor/regex/include
        ${VENDOR_PATH}/ImGuiTextUtils
)

target_link_libraries(FLUF.UI PRIVATE
        "${SDK_PATH}/lib/FLCoreCommon.lib"
        FLUF
        opengl32.lib
)

target_compile_definitions(FLUF.UI PRIVATE
        FLUF_UI
        FLCore
        MLS_LIB_COMPILE
)
add_dependencies(FLUF.UI FLUF)

# DirectX8 SDK
set(DXSDK "${PROJECT_SOURCE_DIR}/vendor/DXSDK")
target_include_directories(FLUF.UI PRIVATE ${DXSDK}/include)
target_link_libraries(FLUF.UI PRIVATE ${DXSDK}/lib/d3d8.lib)

# ImGui
target_compile_definitions(FLUF.UI PRIVATE
        "IMGUI_API=__declspec(dllexport);"
        "STBIDEF=__declspec(dllexport);"
        "FT_FREETYPE_H=<freetype/freetype.h>;"
        "FT_GLYPH_H=<freetype/ftglyph.h>;"
        "FT_MODULE_H=<freetype/ftmodapi.h>;"
        "FT_SYNTHESIS_H=<freetype/ftsynth.h>;"
)
target_link_libraries(FLUF.UI PRIVATE ${VENDOR_PATH}/freetype.lib)
set(IMGUI_IMPL TRUE)

# General
set_general_targets(PROJECT_NAME FLUF.UI)
set_linux_msvc_includes(PROJECT_NAME FLUF.UI)
create_post_build(TARGET_NAME FLUF.UI)

reset_glob()
unset(IMGUI_IMPL)