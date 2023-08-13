# Project setup
project("RAWRBOX.UTILS" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - Utils lib" LANGUAGES CXX C)
set(output_target RAWRBOX.UTILS)

# Grab source files
file(GLOB_RECURSE RAWRBOX_UTILS_IMPORTS "src/*.c" "src/*.cpp" "include/*.hpp" "include/*.h")

# LIBS ---
if(NOT RAWRBOX_ENABLE_QHULL)
    list(REMOVE_ITEM RAWRBOX_UTILS_IMPORTS "include/rawrbox/utils/qhull.h")
    list(REMOVE_ITEM RAWRBOX_UTILS_IMPORTS "tests/qhull.spec.cpp")
else()
    CPMAddPackage(
        NAME
        qhull
        GITHUB_REPOSITORY
        qhull/qhull
        GIT_TAG
        2020.2
        DOWNLOAD_ONLY
        ON)

    # Setup lib
    if(qhull_ADDED)
        set(libqhullr_HEADERS
            ${qhull_SOURCE_DIR}/src/libqhull_r/libqhull_r.h
            ${qhull_SOURCE_DIR}/src/libqhull_r/geom_r.h
            ${qhull_SOURCE_DIR}/src/libqhull_r/io_r.h
            ${qhull_SOURCE_DIR}/src/libqhull_r/mem_r.h
            ${qhull_SOURCE_DIR}/src/libqhull_r/merge_r.h
            ${qhull_SOURCE_DIR}/src/libqhull_r/poly_r.h
            ${qhull_SOURCE_DIR}/src/libqhull_r/qhull_ra.h
            ${qhull_SOURCE_DIR}/src/libqhull_r/qset_r.h
            ${qhull_SOURCE_DIR}/src/libqhull_r/random_r.h
            ${qhull_SOURCE_DIR}/src/libqhull_r/stat_r.h
            ${qhull_SOURCE_DIR}/src/libqhull_r/user_r.h)

        set(libqhullr_SOURCES
            ${qhull_SOURCE_DIR}/src/libqhull_r/global_r.c
            ${qhull_SOURCE_DIR}/src/libqhull_r/stat_r.c
            ${qhull_SOURCE_DIR}/src/libqhull_r/geom2_r.c
            ${qhull_SOURCE_DIR}/src/libqhull_r/poly2_r.c
            ${qhull_SOURCE_DIR}/src/libqhull_r/merge_r.c
            ${qhull_SOURCE_DIR}/src/libqhull_r/libqhull_r.c
            ${qhull_SOURCE_DIR}/src/libqhull_r/geom_r.c
            ${qhull_SOURCE_DIR}/src/libqhull_r/poly_r.c
            ${qhull_SOURCE_DIR}/src/libqhull_r/qset_r.c
            ${qhull_SOURCE_DIR}/src/libqhull_r/mem_r.c
            ${qhull_SOURCE_DIR}/src/libqhull_r/random_r.c
            ${qhull_SOURCE_DIR}/src/libqhull_r/usermem_r.c
            ${qhull_SOURCE_DIR}/src/libqhull_r/userprintf_r.c
            ${qhull_SOURCE_DIR}/src/libqhull_r/io_r.c
            ${qhull_SOURCE_DIR}/src/libqhull_r/user_r.c
            ${qhull_SOURCE_DIR}/src/libqhull_r/rboxlib_r.c
            ${qhull_SOURCE_DIR}/src/libqhull_r/userprintf_rbox_r.c
            ${libqhullr_HEADERS})

        set(qhull_VERSION "8.0.2") # Advance every release
        set(qhull_STATICR qhullstatic_r)

        add_library(${qhull_STATICR} STATIC ${libqhullr_SOURCES})
        set_target_properties(${qhull_STATICR} PROPERTIES VERSION ${qhull_VERSION} OUTPUT_NAME "${qhull_STATICR}$<$<CONFIG:Debug>:d>")
        if(UNIX)
            target_link_libraries(${qhull_STATICR} m)
        endif(UNIX)

        list(APPEND EXTRA_UTIL_LIBS ${qhull_STATICR})
        set(EXTRA_UTIL_INCLUDES "${qhull_SOURCE_DIR}/src/libqhull_r")

        set_lib_runtime_mt(${qhull_STATICR})
    endif()
endif()

if(NOT RAWRBOX_ENABLE_HTTP_REQUEST)
    list(REMOVE_ITEM RAWRBOX_UTILS_IMPORTS "include/rawrbox/utils/http.hpp")
    list(REMOVE_ITEM RAWRBOX_UTILS_IMPORTS "tests/http.spec.cpp")
else()
    CPMAddPackage(
        NAME
        libcpr
        GIT_TAG
        1.10.4
        GITHUB_REPOSITORY
        libcpr/cpr
        OPTIONS
        "BUILD_SHARED_LIBS OFF"
        "CPR_ENABLE_SSL ON"
        "CURL_ZLIB OFF")

    list(APPEND EXTRA_UTIL_LIBS cpr::cpr)
    set_lib_runtime_mt(libcurl)
    set_lib_runtime_mt(cpr)
endif()

CPMAddPackage("gh:bshoshany/thread-pool@3.5.0")
if(thread-pool_ADDED)
    add_library(thread-pool INTERFACE IMPORTED)
    target_include_directories(thread-pool INTERFACE ${thread-pool_SOURCE_DIR}/include)
endif()
# ----

# LIB ----
add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_UTILS_IMPORTS})
target_include_directories(${output_target} PUBLIC "include" ${EXTRA_UTIL_INCLUDES})
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_UTILS)
target_link_libraries(${output_target} PUBLIC ${EXTRA_UTIL_LIBS} thread-pool fmt::fmt)

set_lib_runtime_mt(${output_target})
# --------------

# TEST ----
include(../cmake/catch2.cmake)
# --------------
