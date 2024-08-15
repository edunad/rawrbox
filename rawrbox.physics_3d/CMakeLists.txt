# Project setup
project("RAWRBOX.PHYSICS.3D" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - 3D PHYSICS lib" LANGUAGES CXX)
set(output_target RAWRBOX.PHYSICS.3D)

# LIBS ---
set(CMAKE_CXX_ORIGINAL_FLAGS ${CMAKE_CXX_FLAGS})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DEBUG_RENDERER_IN_DEBUG_AND_RELEASE ON CACHE STRING "" FORCE)
    message(STATUS "Enabled jolt debug renderer")
else()
    set(DEBUG_RENDERER_IN_DEBUG_AND_RELEASE OFF CACHE STRING "" FORCE)
    message(STATUS "Disabled jolt debug renderer")
endif()

set(CPP_EXCEPTIONS_ENABLED ON CACHE STRING "" FORCE)
set(CPP_RTTI_ENABLED ON CACHE STRING "" FORCE)

CPMAddPackage(
    NAME
        Jolt
    GITHUB_REPOSITORY
        jrouwe/JoltPhysics
    VERSION
        5.1.0
    OPTIONS
        "INTERPROCEDURAL_OPTIMIZATION ${RAWRBOX_INTERPROCEDURAL_OPTIMIZATION}"

        "COMPILE_AS_SHARED_LIBRARY OFF"
        "USE_STATIC_MSVC_RUNTIME_LIBRARY ON"

        "ENABLE_ALL_WARNINGS OFF"

        "DEBUG_RENDERER_IN_DISTRIBUTION OFF"

        "PROFILER_IN_DISTRIBUTION OFF"
        "PROFILER_IN_DEBUG_AND_RELEASE OFF"

    SOURCE_SUBDIR
        "Build"
)
# ----

# LIB ----
file(GLOB_RECURSE RAWRBOX_PHYSICS_IMPORTS "src/*.cpp" "include/*.hpp")
add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_PHYSICS_IMPORTS})
target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_PHYSICS_3D)
target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})

if(DEBUG_RENDERER_IN_DEBUG_AND_RELEASE AND RAWRBOX_BUILD_RAWRBOX_RENDER)
    target_link_libraries(${output_target} PUBLIC RAWRBOX.RENDER Jolt)
else()
    target_link_libraries(${output_target} PUBLIC RAWRBOX.ENGINE RAWRBOX.MATH Jolt)
endif()

set_lib_runtime_mt(${output_target})
# --------------

# TEST ----
#include(../cmake/catch2.cmake)
# --------------
