# Project setup
project(
    "RAWRBOX.PHYSICS"
    VERSION ${RAWRBOX_VERSION}
    DESCRIPTION "RawrBox - PHYSICS lib"
    LANGUAGES CXX)
set(output_target RAWRBOX.PHYSICS)

# LIBS ---
CPMAddPackage(
    NAME
    Jolt
    GITHUB_REPOSITORY
    jrouwe/JoltPhysics
    VERSION
    3.0.1
    OPTIONS
    # "CROSS_PLATFORM_DETERMINISTIC ON"
    "COMPILE_AS_SHARED_LIBRARY OFF"
    "INTERPROCEDURAL_OPTIMIZATION ON"
    "USE_STATIC_MSVC_RUNTIME_LIBRARY ON"
    #"TARGET_SAMPLES ON"
    "ENABLE_ALL_WARNINGS OFF"
    SOURCE_SUBDIR
    "Build")
# ----

# LIB ----
file(GLOB_RECURSE RAWRBOX_PHYSICS_IMPORTS "src/*.cpp" "include/*.hpp")
add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_PHYSICS_IMPORTS})
target_compile_definitions(
    ${output_target}
    PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX
    INTERFACE RAWRBOX_PHYSICS)
target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC RAWRBOX.ENGINE RAWRBOX.MATH Jolt magic_enum::magic_enum)

set_lib_runtime_mt(${output_target})
# --------------

# TEST ----
#include(../cmake/catch2.cmake)
# --------------
