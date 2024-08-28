# Project setup
project("RAWRBOX.PHYSICS.2D" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - 2D PHYSICS lib" LANGUAGES CXX)
set(output_target RAWRBOX.PHYSICS.2D)

# LIBS ---
CPMAddPackage(
    NAME
        muli
    GITHUB_REPOSITORY
        Sopiro/Muli
    GIT_TAG
        "c7697eaee46d128b967368d7454aa2459fed15d4" # master branch is currently broken: https://github.com/Sopiro/Muli/issues/3
    OPTIONS
        "MULI_BUILD_DEMO OFF")

set_lib_runtime_mt(muli)
# ----

# LIB ----
file(GLOB_RECURSE RAWRBOX_PHYSICS_IMPORTS "src/*.cpp" "include/*.hpp")
add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_PHYSICS_IMPORTS})
target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_PHYSICS_2D)
target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC RAWRBOX.ENGINE RAWRBOX.MATH muli)

set_lib_runtime_mt(${output_target})
# --------------

# TEST ----
#include(../cmake/catch2.cmake)
# --------------
