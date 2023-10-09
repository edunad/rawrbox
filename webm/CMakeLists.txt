# Project setup
project("RAWRBOX.WEBM" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - WEBM lib" LANGUAGES CXX)
set(output_target RAWRBOX.WEBM)
# --------------

# LIBS ---
CPMAddPackage(
    NAME
    libwebm
    GIT_TAG
    libwebm-1.0.0.30
    GITHUB_REPOSITORY
    webmproject/libwebm
    OPTIONS
    "BUILD_SHARED_LIBS OFF"
    "ENABLE_WEBMTS OFF"
    "ENABLE_WEBMINFO OFF"
    "ENABLE_WEBM_PARSER OFF"
    "ENABLE_TESTS OFF"
    "ENABLE_SAMPLE_PROGRAMS OFF")

if(libwebm_ADDED)
    target_include_directories(webm PUBLIC ${libwebm_SOURCE_DIR})
    set_lib_runtime_mt(webm)
    set_lib_runtime_mt(mkvparser)

    # Add codec
    set(VPX_VERSION v1.13.0)
    set(VPX_MSC_VERSION 17)
    if(RAWRBOX_BUILD_MSVC_MULTITHREADED_RUNTIME)
        set(VPX_STATIC_MULTITHREADED ON)
    endif()

    find_package(VPX REQUIRED)

    if(WIN32)
        add_custom_target(
            copy_dll_vpx ALL COMMAND ${CMAKE_COMMAND} -E copy ${VPX_BINARY}
                                     ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}
            COMMENT "Copying VPX DLL into binary directory")
    endif()
    # ----
endif()

# Grab source files
file(GLOB_RECURSE RAWRBOX_WEBM_IMPORTS "src/*.cpp" "include/*.hpp")

if(RAWRBOX_BUILD_RAWRBOX_RESOURCES)
    set(RAWRBOX_EXTRA_LIBS RAWRBOX.RESOURCES)
else()
    remove_folder("${RAWRBOX_WEBM_IMPORTS}" "/resources/")
    set(RAWRBOX_WEBM_IMPORTS "${CLEAN_SOURCES}")
endif()
# ----

add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_WEBM_IMPORTS})
if(WIN32)
    add_dependencies(${output_target} copy_dll_vpx)
endif()
target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_WEBM)
target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC RAWRBOX.RENDER_TEMP ${RAWRBOX_EXTRA_LIBS} VPX::VPX webm)

set_lib_runtime_mt(${output_target})
# --------------

# TEST ----
#include(../cmake/catch2.cmake)
# --------------
