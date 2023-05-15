
# Project setup
project ("RAWRBOX.UTILS" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - Utils lib" LANGUAGES CXX)
set(output_target RAWRBOX.UTILS)


# Grab source files
file(GLOB_RECURSE RAWRBOX_UTILS_IMPORTS "src/*.c" "src/*.cpp" "include/*.hpp" "include/*.h")

if(NOT RAWRBOX_ENABLE_QHULL)
	list(REMOVE_ITEM RAWRBOX_UTILS_IMPORTS "include/rawrbox/utils/qhull.h")
else()
	find_package(Qhull REQUIRED)
	set(EXTRA_UTIL_LIBS Qhull::qhullstatic_r)
endif()

# LIB ----

add_library(${output_target} ${RAWRBOX_UTILS_IMPORTS})
target_include_directories(${output_target} PUBLIC "include" )
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_compile_definitions(${output_target} PUBLIC _CRT_SECURE_NO_WARNINGS NOMINMAX RAWRBOX_UTILS)
target_link_libraries(${output_target} PUBLIC ${EXTRA_UTIL_LIBS})
#--------------


# TEST ----
include(../cmake/catch2.cmake)
#--------------
