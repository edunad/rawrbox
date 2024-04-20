# Project setup
project("RAWRBOX.UI" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - UI lib" LANGUAGES CXX)
set(output_target RAWRBOX.UI)
# --------------

# LIBS ---
CPMAddPackage(
    NAME
        clip
    VERSION
        1.9
    GITHUB_REPOSITORY
        dacap/clip
    OPTIONS
        "CLIP_EXAMPLES OFF"
        "CLIP_ENABLE_IMAGE OFF"
        "CLIP_TESTS OFF")

if(clip_ADDED)
    target_include_directories(clip PUBLIC ${clip_SOURCE_DIR})
    set_lib_runtime_mt(clip)
endif()

# ----

# Grab source files
file(GLOB_RECURSE RAWRBOX_UI_IMPORTS "src/*.cpp" "src/*.c" "include/*.h" "include/*.hpp")
if(NOT RAWRBOX_BUILD_RAWRBOX_SCRIPTING)
    remove_folder("${RAWRBOX_UI_IMPORTS}" "/scripting/")
    set(RAWRBOX_UI_IMPORTS "${CLEAN_SOURCES}")
endif()

if(NOT RAWRBOX_BUILD_RAWRBOX_RESOURCES)
    message(FATAL "RAWRBOX.UI elements require RAWRBOX.RESOURCES to be enabled!")
endif()
# -----------------

# LIB ----
add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_UI_IMPORTS})

copy_resources(TARGET ${output_target})
if(RAWRBOX_BUILD_RAWRBOX_SCRIPTING)
    copy_lua_libs(TARGET ${output_target})
endif()

target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_UI)
target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC RAWRBOX.RENDER ${RAWRBOX_EXTRA_LIBS} clip)

set_lib_runtime_mt(${output_target})
# --------------

# TEST ----
#include(../cmake/catch2.cmake)
# --------------
