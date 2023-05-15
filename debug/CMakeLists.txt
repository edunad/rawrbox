
# Project setup
project ("RAWRBOX.DEBUG" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - Debug util lib" LANGUAGES CXX)
set(output_target RAWRBOX.DEBUG)
#--------------

if(RAWRBOX_ENABLE_BASS_SUPPORT)
    set(RAWRBOX_DEBUG_DEFINES RAWRBOX_BASS)
endif()




# LIB ----
file(GLOB_RECURSE RAWRBOX_DEBUG_IMPORTS "include/*.hpp" "src/*.cpp" )
add_library(${output_target} ${RAWRBOX_DEBUG_IMPORTS})
target_compile_definitions(${output_target} PUBLIC _CRT_SECURE_NO_WARNINGS NOMINMAX ${RAWRBOX_DEBUG_DEFINES} RAWRBOX_DEBUG)
target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC RAWRBOX.RENDER)
#--------------


# TEST ----
include(../cmake/catch2.cmake)
#--------------
