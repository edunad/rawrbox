# Project setup
project(
    "RAWRBOX.UTILS"
    VERSION ${RAWRBOX_VERSION}
    DESCRIPTION "RawrBox - Utils lib"
    LANGUAGES CXX)
set(output_target RAWRBOX.UTILS)

# Grab source files
file(GLOB_RECURSE RAWRBOX_UTILS_IMPORTS "src/*.c" "src/*.cpp" "include/*.hpp" "include/*.h")
list(REMOVE_ITEM RAWRBOX_UTILS_IMPORTS "include/rawrbox/utils/qhull.h")

#
# if(NOT RAWRBOX_ENABLE_QHULL)
#list(REMOVE_ITEM RAWRBOX_UTILS_IMPORTS "include/rawrbox/utils/qhull.h")
# else()
# CPMAddPackage( NAME Qhull GITHUB_REPOSITORY qhull/qhull GIT_TAG 2020.2 OPTIONS "QHULL_ENABLE_TESTING OFF" "BUILD_STATIC_LIBS ON" "BUILD_SHARED_LIBS OFF" )
#
# set(EXTRA_UTIL_LIBS qhullstatic_r)
#endif()

# LIBS ---

CPMAddPackage("gh:bshoshany/thread-pool@3.5.0")
if(thread-pool_ADDED)
    add_library(thread-pool INTERFACE IMPORTED)
    target_include_directories(thread-pool INTERFACE ${thread-pool_SOURCE_DIR}/include)
endif()
# ----

# LIB ----
add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_UTILS_IMPORTS})
target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_UTILS)
target_link_libraries(${output_target} PUBLIC ${EXTRA_UTIL_LIBS} thread-pool fmt::fmt)

set_lib_runtime_mt(${output_target})
# --------------

# TEST ----
include(../cmake/catch2.cmake)
# --------------
