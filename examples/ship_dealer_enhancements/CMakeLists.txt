cmake_minimum_required(VERSION 3.23.5)

set(VER_MAJOR 1)
set(VER_MINOR 0)
set(VER_PATCH 0)
set(NAME "Ship Dealer Enhancements")
set(DESCRIPTION "Causes ship dealers to paginate their shop pages, bypassing the limit of 3 ships. Also causes the shop to respect the order of ships in the market_ships.ini")
set(SHIP_DEALER_ENHANCEMENTS_PATH ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

# Glob our source files and header files
begin_glob()

configure_file(${ROOT_DIR}/metadata.rc.in ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc @ONLY)
add_library(ship_dealer_enhancements SHARED ${SOURCE_FILES} ${ROOT_DIR}/PCH.cpp ${ROOT_DIR}/PCH.hpp ${CMAKE_CURRENT_BINARY_DIR}/metadata.rc)

# Dependencies
target_include_directories(ship_dealer_enhancements PRIVATE ${RML_PATH}/Include)
target_include_directories(ship_dealer_enhancements PRIVATE ${FLUF_UI_PATH}/Include)
add_dependencies(ship_dealer_enhancements FLUF FLUF.UI)
target_link_libraries(ship_dealer_enhancements PRIVATE FLUF FLUF.UI)

# Link against FLCore
target_link_libraries(ship_dealer_enhancements PRIVATE "${SDK_PATH}/lib/FLCoreCommon.lib")

target_precompile_headers(ship_dealer_enhancements PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${ROOT_DIR}/PCH.hpp>")

set_general_targets(PROJECT_NAME ship_dealer_enhancements)
set_linux_msvc_includes(PROJECT_NAME ship_dealer_enhancements)
create_post_build(TARGET_NAME ship_dealer_enhancements)

reset_glob()
