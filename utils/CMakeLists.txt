
# Project setup
project ("RAWRBOX.UTILS" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - Utils lib" LANGUAGES CXX)
set(output_target RAWRBOX.UTILS)


# Grab source files
file(GLOB_RECURSE RAWRBOX_UTILS_IMPORTS "src/*.c" "include/*.hpp" "include/*.h")

if(NOT RAWRBOX_ENABLE_QHULL)
	list(REMOVE_ITEM RAWRBOX_UTILS_IMPORTS "include/rawrbox/utils/qhull.h")
else()
	find_package(Qhull REQUIRED)
	set(EXTRA_UTIL_LIBS Qhull::qhullstatic_r)
endif()

# LIB ----
add_library(${output_target} INTERFACE ${RAWRBOX_UTILS_IMPORTS})
target_include_directories(${output_target} INTERFACE "include")
target_link_libraries(${output_target} INTERFACE ${EXTRA_UTIL_LIBS})
#--------------


# TEST ----
include(../cmake/catch2.cmake)
#--------------
