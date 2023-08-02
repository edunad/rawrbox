# Project setup
project(
    "RAWRBOX.MATH"
    VERSION ${RAWRBOX_VERSION}
    DESCRIPTION "RawrBox - Math lib"
    LANGUAGES CXX)
set(output_target RAWRBOX.MATH)

# LIB ----
file(GLOB_RECURSE RAWRBOX_MATH_IMPORTS "src/*.c" "src/*.cpp" "include/*.hpp" "include/*.h")

add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_MATH_IMPORTS})
target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_MATH)

set_lib_runtime_mt(${output_target})
# --------------


# TEST ----
include(../cmake/catch2.cmake)
# --------------
