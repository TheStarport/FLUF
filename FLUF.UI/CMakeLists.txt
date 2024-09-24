begin_glob()

# Conditionally add debug flags
add_compile_options("$<$<CONFIG:DEBUG>:-D_DEBUG>")

add_library(FLUF.UI SHARED ${SOURCE_FILES})

target_include_directories(FLUF.UI PRIVATE ${INCLUDE_PATH})
target_include_directories(FLUF.UI PRIVATE ${SDK_PATH}/include)
target_include_directories(FLUF.UI PRIVATE ${SDK_PATH}/vendor)
target_include_directories(FLUF.UI PRIVATE ${REFLECT_PATH}/include)
target_include_directories(FLUF.UI PRIVATE ${REFLECT_PATH}/include/rfl/thirdparty)
target_include_directories(FLUF.UI PRIVATE ${RML_PATH}/Include)

target_link_libraries(FLUF.UI PRIVATE "${SDK_PATH}/lib/FLCoreCommon.lib")

set_linux_msvc_includes(PROJECT_NAME FLUF.UI)

#target_precompile_headers(FLUF.UI PRIVATE ${INCLUDE_PATH}/PCH.hpp)

reset_glob()