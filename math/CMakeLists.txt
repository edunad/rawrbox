
# Project setup
project ("RAWRBOX.MATH" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - Math lib" LANGUAGES CXX)
set(output_target RAWRBOX.MATH)

# LIB ----
file(GLOB_RECURSE RAWRBOX_MATH_IMPORTS "include/*.h" "include/*.hpp" )
add_library(${output_target} INTERFACE ${RAWRBOX_MATH_IMPORTS})
target_include_directories(${output_target} INTERFACE "include")

if (("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows" OR "${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore") AND NOT MINGW)
    set_property(TARGET ${output_target} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
#--------------

# TEST ----
include(../cmake/catch2.cmake)
#--------------
