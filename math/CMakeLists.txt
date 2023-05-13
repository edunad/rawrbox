
# Project setup
project ("RAWRBOX.MATH" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - Math lib" LANGUAGES CXX)
set(output_target RAWRBOX.MATH)

# LIB ----
file(GLOB_RECURSE RAWRBOX_MATH_IMPORTS "include/*.h" "include/*.hpp" )
add_library(${output_target} INTERFACE ${RAWRBOX_MATH_IMPORTS})
target_include_directories(${output_target} INTERFACE "include")
#--------------

# TEST ----
include(../cmake/catch2.cmake)
#--------------
