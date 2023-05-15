

# Project setup
project ("RAWRBOX.RESOURCES" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - Resources manager lib" LANGUAGES CXX)
set(output_target RAWRBOX.RESOURCES)

# LIB ----
file(GLOB_RECURSE RAWRBOX_RESOURCES_IMPORTS "src/*.cpp" "include/*.hpp")
add_library(${output_target} ${RAWRBOX_RESOURCES_IMPORTS})

target_include_directories(${output_target} PUBLIC "include" )
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_compile_definitions(${output_target} PUBLIC _CRT_SECURE_NO_WARNINGS NOMINMAX RAWRBOX_RESOURCES)
target_link_libraries(${output_target} PUBLIC RAWRBOX.UTILS fmt::fmt)
#--------------

# TEST ----
#include(../cmake/catch2.cmake)
#--------------