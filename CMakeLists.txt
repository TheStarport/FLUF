cmake_minimum_required(VERSION 3.23)
project("FLUF" LANGUAGES C CXX)

find_program(CCACHE_FOUND ccache)
if (CCACHE_FOUND)
    message(STATUS "Found CCache, applying rules")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
endif ()

include("LoadConf.cmake")

if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
    add_compile_definitions("WINAPI_FAMILY=100")
    if (NOT DEFINED ENV{MSVC})
        message(FATAL_ERROR "MSVC env was not found and running on Linux")
    endif ()

    if (NOT DEFINED ENV{INCLUDE})
        message(FATAL_ERROR "'INCLUDE' env not found. Have you run 'source $ENV{MSVC}/bin/x86/msvcenv.sh' ?")
    endif ()

    string(REPLACE "z:\\" "/" MSVC_INCLUDE "$ENV{INCLUDE}")
    string(REPLACE "\\" "/" MSVC_INCLUDE "${MSVC_INCLUDE}")
    string(REPLACE ";" " " MSVC_INCLUDE "${MSVC_INCLUDE}")
    separate_arguments(MSVC_INCLUDE)
    include_directories(${MSVC_INCLUDE})
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D /SAFESEH:NO")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO")

message(STATUS "Detected compiler: ${CMAKE_CXX_COMPILER}")

set(PROJECT_VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 20)

add_compile_options("$<$<CONFIG:DEBUG>:-D_DEBUG>")

if (DEFINED NO_HOOKS)
    add_definitions("-DFLUF_DISABLE_HOOKS")
endif ()

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(FATAL_ERROR "Only LLVM Clang and MSVC compilers is currently supported.")
endif ()

set(ROOT_DIR "${PROJECT_SOURCE_DIR}")
set(VENDOR_PATH "${PROJECT_SOURCE_DIR}/vendor")
set(SDK_PATH "${VENDOR_PATH}/FLHookSDK")
set(REFLECT_PATH "${VENDOR_PATH}/reflect-cpp")

set(Python_FIND_VIRTUALENV FIRST)
find_package(Python 3.11 REQUIRED)

# Libs
set(RML_PATH "${VENDOR_PATH}/RmlUi")
set(CURL_PATH "${VENDOR_PATH}/curl-8.10.1")
set(IMGUI_PATH "${VENDOR_PATH}/imgui")

# Set Freetype requirements for RML
set(FREETYPE_LIBRARY "${VENDOR_PATH}/freetype.lib")
set(FREETYPE_INCLUDE_DIRS "${VENDOR_PATH}/freetype/include")

# Import Conan Packages
find_package(glm REQUIRED)
find_package(magic_enum REQUIRED)
find_package(msgpack REQUIRED)
find_package(lua REQUIRED)
find_package(xbyak REQUIRED)
find_package(stb REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(lunasvg REQUIRED)

# Compile RML w/ Lua and SVG support
set(RMLUI_LUA_BINDINGS TRUE)
set(RML_LUA_BINDINGS_LIBRARY lua)
set(RMLUI_SVG_PLUGIN ON)
set(LUA_INCLUDE_DIR ${lua_INCLUDE_DIR})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(lunasvg_ROOT "${lunasvg_LIB_DIRS_DEBUG}")
    set(LUA_LIBRARIES "${lua_LIB_DIRS_DEBUG}/lua.lib")
else ()
    set(lunasvg_ROOT "${lunasvg_LIB_DIRS_RELEASE}")
    set(LUA_LIBRARIES "${lua_LIB_DIRS_RELEASE}/lua.lib")
endif ()

set(REFLECTCPP_YAML ON)
set(REFLECTCPP_BUILD_SHARED OFF)
add_subdirectory(${REFLECT_PATH})
add_subdirectory(${RML_PATH})
target_include_directories(rmlui_core PUBLIC ${lunasvg_INCLUDE_DIRS}/lunasvg)

function(set_linux_msvc_includes)
    cmake_parse_arguments(
            PARSED_ARGS # prefix of output variables
            ""
            "PROJECT_NAME" # list of names of mono-valued arguments
            ""
            ${ARGN} # arguments of the function to parse, here we take the all original ones
    )

    if (NOT PARSED_ARGS_PROJECT_NAME)
        message(FATAL_ERROR "set_linux_msvc_includes called without specifying a project name")
    endif ()
endfunction()

function(set_general_targets)
    cmake_parse_arguments(
            PARSED_ARGS # prefix of output variables
            ""
            "PROJECT_NAME" # list of names of mono-valued arguments
            ""
            ${ARGN} # arguments of the function to parse, here we take the all original ones
    )

    if (NOT PARSED_ARGS_PROJECT_NAME)
        message(FATAL_ERROR "set_general_targets called without specifying a project name")
    endif ()

    target_include_directories(${PARSED_ARGS_PROJECT_NAME} PRIVATE ${ROOT_DIR} ${ROOT_DIR}/FLUF/Include)
    target_include_directories(${PARSED_ARGS_PROJECT_NAME} PRIVATE ${INCLUDE_PATH})
    target_include_directories(${PARSED_ARGS_PROJECT_NAME} PRIVATE ${SDK_PATH}/include)
    target_include_directories(${PARSED_ARGS_PROJECT_NAME} PRIVATE ${SDK_PATH}/vendor)
    target_include_directories(${PARSED_ARGS_PROJECT_NAME} PRIVATE ${REFLECT_PATH}/include)
    target_include_directories(${PARSED_ARGS_PROJECT_NAME} PRIVATE ${REFLECT_PATH}/include/rfl/thirdparty)

    target_include_directories(${PARSED_ARGS_PROJECT_NAME} PRIVATE
            ${IMGUI_PATH}
            ${IMGUI_PATH}/misc/cpp
            ${IMGUI_PATH}/freetype
    )

    target_compile_definitions(${PARSED_ARGS_PROJECT_NAME} PRIVATE
            "_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING;"
            "_CRT_SECURE_NO_WARNINGS;"
            "WIN32;"
            "WIN32_LEAN_AND_MEAN;"
            "UNICODE;_UNICODE;"
            "USE_GLM;"
            "NOMINMAX;"
            "IMGUI_DEFINE_MATH_OPERATORS;"
            "IMGUI_ENABLE_FREETYPE;"
            "IMGUI_USE_STB_SPRINTF;"
            "IMGUI_USE_WCHAR32;"
    )

    if (NOT DEFINED IMGUI_IMPL)
        target_compile_definitions(${PARSED_ARGS_PROJECT_NAME} PRIVATE
                "IMGUI_API=__declspec(dllimport);"
                "STBIDEF=__declspec(dllimport)"
        )
    endif ()

    add_dependencies(${PARSED_ARGS_PROJECT_NAME} reflectcpp)

    target_link_libraries(${PARSED_ARGS_PROJECT_NAME} PRIVATE
            reflectcpp
            glm::glm
            magic_enum::magic_enum
            stb::stb
            xbyak::xbyak
            yaml-cpp::yaml-cpp
            msgpack-c
    )

    # Selectively skip PCH for certain files
    set_source_files_properties(
            ${REFLECT_PATH}/src/yyjson.c
            ${REFLECT_PATH}/src/reflectcpp.cpp
            PROPERTIES SKIP_PRECOMPILED_HEADERS ON
    )

    set_target_properties(${PARSED_ARGS_PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
    target_precompile_headers(${PARSED_ARGS_PROJECT_NAME} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${ROOT_DIR}/PCH.hpp>")
endfunction()

function(begin_glob)
    set(SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Source)
    set(INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Include)
    file(GLOB_RECURSE TMP_FILES LIST_DIRECTORIES false
            "${SOURCE_PATH}/*.c*"
            "${SOURCE_PATH}/*.h*"
            "${INCLUDE_PATH}/*.c*"
            "${INCLUDE_PATH}/*.h*"
            "${SDK_PATH}/*.h*"
    )
    set(SOURCE_PATH ${SOURCE_PATH} PARENT_SCOPE)
    set(INCLUDE_PATH ${INCLUDE_PATH} PARENT_SCOPE)
    set(SOURCE_FILES ${TMP_FILES} PARENT_SCOPE)
endfunction()

function(reset_glob)
    unset(SOURCE_PATH PARENT_SCOPE)
    unset(INCLUDE_PATH PARENT_SCOPE)
    unset(SOURCE_FILES PARENT_SCOPE)
endfunction()

function(create_post_build)
    cmake_parse_arguments(
            PARSED_ARGS # prefix of output variables
            ""
            "TARGET_NAME" # list of names of mono-valued arguments
            ""
            ${ARGN} # arguments of the function to parse, here we take the all original ones
    )

    if (NOT DEFINED NO_POST_BUILD)
        if (CMAKE_BUILD_TYPE STREQUAL "Release")
            set(RELEASE_FLAG "--release")
        endif ()
        if (DEFINED BINARY_COPY_DESTINATION)
            set(DEST_FLAG --dest=\"${BINARY_COPY_DESTINATION}\")
        endif ()

        add_custom_command(TARGET ${PARSED_ARGS_TARGET_NAME} POST_BUILD
                COMMAND ${Python_EXECUTABLE} ${ROOT_DIR}/cli.py post-build ${RELEASE_FLAG} ${DEST_FLAG} WORKING_DIRECTORY ${ROOT_DIR}
        )
    endif ()
endfunction()

# Now we add our sub-projects

add_subdirectory(FLUF) # FLUF Core
add_subdirectory(FLUF.UI) # FLUF user interface module
add_subdirectory(FLUF.CrashWalker) # FLUF anti-crash protections

if (BUILD_SAMPLES)
    file(GLOB V_GLOB LIST_DIRECTORIES true "examples/*")
    foreach (ENTRY ${V_GLOB})
        get_filename_component(DIR ${ENTRY} NAME_WLE)
        if (IS_DIRECTORY ${ENTRY} AND NOT ${DIR} STREQUAL "shared_assets")
            message(STATUS "Configuring Example: ${DIR}")
            add_subdirectory(${ENTRY})
        endif ()
    endforeach ()
endif ()
