cmake_minimum_required(VERSION 3.23)
project("FLUF" LANGUAGES C CXX)

find_program(CCACHE_FOUND ccache)
if (CCACHE_FOUND)
    message(STATUS "Found CCache, applying rules")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D \"SAFESEH:NO\"")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO")

message(STATUS "Detected compiler: ${CMAKE_CXX_COMPILER}")

set(PROJECT_VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 20)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(FATAL_ERROR "Only LLVM Clang and MSVC compilers is currently supported.")
endif ()

set(ROOT_DIR "${PROJECT_SOURCE_DIR}")
set(VENDOR_PATH "${PROJECT_SOURCE_DIR}/vendor")
set(SDK_PATH "${VENDOR_PATH}/FLHookSDK")
set(REFLECT_PATH "${VENDOR_PATH}/reflect-cpp")

# Python for post build
find_package(Python 3.11 REQUIRED)
if (EXISTS "${ROOT_DIR}/.venv")
    set(ENV{VIRTUAL_ENV} "${ROOT_DIR}/.venv")
elseif ((EXISTS "${ROOT_DIR}/venv"))
    set(ENV{VIRTUAL_ENV} "${ROOT_DIR}/venv")
else ()
    message(STATUS "venv not found, creating and installing packages")
    execute_process(COMMAND "${Python_EXECUTABLE}" -m venv "${ROOT_DIR}/venv")
    if (EXISTS "${ROOT_DIR}/venv/Scripts")
        execute_process(COMMAND "${ROOT_DIR}/venv/Scripts/pip.exe" install click conan)
    else ()
        execute_process(COMMAND "${ROOT_DIR}/venv/bin/pip" install click conan)
    endif ()
    set(ENV{VIRTUAL_ENV} "${ROOT_DIR}/venv")

endif ()

set(Python_FIND_VIRTUALENV FIRST)
unset(Python_EXECUTABLE)
find_package(Python 3.11 REQUIRED)

# Libs
set(RML_PATH "${VENDOR_PATH}/RmlUi")
set(CURL_PATH "${VENDOR_PATH}/curl-8.10.1")

# Set Freetype requirements for RML
set(FREETYPE_LIBRARY "${VENDOR_PATH}/freetype.lib")
set(FREETYPE_INCLUDE_DIRS "${VENDOR_PATH}/freetype/include")

# Import Conan Packages
find_package(glm REQUIRED)
find_package(magic_enum REQUIRED)
find_package(lua REQUIRED)
find_package(yaml-cpp REQUIRED)

# Compile RML w/ Lua support
set(RMLUI_LUA_BINDINGS TRUE)
set(RML_LUA_BINDINGS_LIBRARY lua)
set(LUA_INCLUDE_DIR ${lua_INCLUDE_DIR})

if (CMAKE_BUILD_TYPE_INIT STREQUAL "Debug")
    set(LUA_LIBRARIES ${lua_LIB_DIRS_DEBUG}/lua.lib)
else ()
    set(LUA_LIBRARIES ${lua_LIB_DIRS_RELEASE}/lua.lib)
endif ()

add_subdirectory(${RML_PATH})

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

    # If we are building on Linux, import the MSVC headers
    if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
        if (NOT DEFINED ENV{MSVC_INCLUDE})
            message(FATAL_ERROR "MSVC_INCLUDE was not found and running on Linux")
        endif ()
        string(REPLACE ":" " " MSVC_INCLUDES $ENV{MSVC_INCLUDE})
        separate_arguments(MSVC_INCLUDES)
        target_include_directories(${PARSED_ARGS_PROJECT_NAME} PRIVATE ${MSVC_INCLUDES})
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

    target_include_directories(${PARSED_ARGS_PROJECT_NAME} PRIVATE ${ROOT_DIR})
    target_include_directories(${PARSED_ARGS_PROJECT_NAME} PRIVATE ${INCLUDE_PATH})
    target_include_directories(${PARSED_ARGS_PROJECT_NAME} PRIVATE ${SDK_PATH}/include)
    target_include_directories(${PARSED_ARGS_PROJECT_NAME} PRIVATE ${SDK_PATH}/vendor)
    target_include_directories(${PARSED_ARGS_PROJECT_NAME} PRIVATE ${REFLECT_PATH}/include)
    target_include_directories(${PARSED_ARGS_PROJECT_NAME} PRIVATE ${REFLECT_PATH}/include/rfl/thirdparty)

    target_compile_definitions(${PARSED_ARGS_PROJECT_NAME} PRIVATE
            "_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING;"
            "_CRT_SECURE_NO_WARNINGS;"
            "WIN32;"
            "WIN32_LEAN_AND_MEAN;"
            "UNICODE;_UNICODE;"
            "USE_GLM;"
            "NOMINMAX;"
    )

    target_link_libraries(${PARSED_ARGS_PROJECT_NAME} PRIVATE
            glm::glm
            magic_enum::magic_enum
            yaml-cpp::yaml-cpp
    )
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
            "${REFLECT_PATH}/src/*.c*"
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

    add_custom_command(TARGET ${PARSED_ARGS_TARGET_NAME} POST_BUILD
            COMMAND ${Python_EXECUTABLE} ${ROOT_DIR}/cli.py post-build WORKING_DIRECTORY ${ROOT_DIR}
    )
endfunction()

# Now we add our sub-projects

add_subdirectory(FLUF.UI)