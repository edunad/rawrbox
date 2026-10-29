# Project setup
project("RAWRBOX.UTILS" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - Utils lib" LANGUAGES CXX C)
set(output_target RAWRBOX.UTILS)

# Grab source files
file(GLOB_RECURSE RAWRBOX_UTILS_IMPORTS "src/*.c" "src/*.cpp" "include/*.hpp" "include/*.h")

# LIBS ---
if (NOT RAWRBOX_BUILD_QHULL)
    list(REMOVE_ITEM RAWRBOX_UTILS_IMPORTS "include/rawrbox/utils/qhull.h")
else ()
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
    if (qhull_ADDED)
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
        if (UNIX)
            target_link_libraries(${qhull_STATICR} m)
        endif (UNIX)

        list(APPEND EXTRA_UTIL_LIBS ${qhull_STATICR})
        set(EXTRA_UTIL_INCLUDES "${qhull_SOURCE_DIR}/src/libqhull_r")

        set_lib_runtime_mt(${qhull_STATICR})
    endif ()
endif ()

CPMAddPackage("gh:bshoshany/thread-pool@5.1.0")
if (thread-pool_ADDED)
    add_library(thread-pool INTERFACE IMPORTED)
    target_include_directories(thread-pool INTERFACE ${thread-pool_SOURCE_DIR}/include)
endif ()

CPMAddPackage(
        NAME
        spdlog
        GITHUB_REPOSITORY
        gabime/spdlog
        GIT_TAG
        d6c93f38a08b4ea0f8ffcfd7f7857307d5d5c3fa # latest breaks fmt
        OPTIONS
        "SPDLOG_FMT_EXTERNAL ON"
)

if (spdlog_ADDED)
    set_lib_runtime_mt(spdlog)
endif ()

CPMAddPackage(
        NAME
        zstd
        GITHUB_REPOSITORY
        facebook/zstd
        VERSION
        1.5.7
        SOURCE_SUBDIR
        build/cmake
        OPTIONS
        "ZSTD_BUILD_PROGRAMS OFF"
        "ZSTD_BUILD_SHARED OFF"
        "ZSTD_BUILD_STATIC ON"
        "ZSTD_BUILD_TESTS OFF"
        "ZSTD_LEGACY_SUPPORT OFF"
        "ZSTD_MULTITHREAD_SUPPORT OFF"
)

if (zstd_ADDED)
    set_lib_runtime_mt(libzstd_static)
endif ()

CPMAddPackage(
        NAME
        tracy
        GITHUB_REPOSITORY
        wolfpld/tracy
        VERSION
        0.14.1
        OPTIONS
        "TRACY_ENABLE ${RAWRBOX_BUILD_PROFILER}"
        "TRACY_ONLY_LOCALHOST ON"
)

if (tracy_ADDED)
    set_lib_runtime_mt(TracyClient)
endif ()

if (RAWRBOX_BUILD_PROFILER)
    message(STATUS "Enabled TRACY profiler support")

    if (WIN32)
        set(TRACY_PROFILER_PLATFORM "windows")
    elseif (APPLE)
        set(TRACY_PROFILER_PLATFORM "macos")
    else ()
        set(TRACY_PROFILER_PLATFORM "linux")
    endif ()

    CPMAddPackage(
            NAME
            tracy-profiler
            URL
            "https://github.com/wolfpld/tracy/releases/download/v${CPM_PACKAGE_tracy_VERSION}/${TRACY_PROFILER_PLATFORM}-${CPM_PACKAGE_tracy_VERSION}.zip"
            DOWNLOAD_ONLY
            ON)
endif ()
# ----

# LIB ----
add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_UTILS_IMPORTS})

target_include_directories(${output_target} PUBLIC "include" ${EXTRA_UTIL_INCLUDES})
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_UTILS)
target_link_libraries(${output_target} PUBLIC
        ${EXTRA_UTIL_LIBS}

        thread-pool
        spdlog
        libzstd_static
        fmt::fmt
        glaze::glaze
        magic_enum::magic_enum
        Tracy::TracyClient
)

set_lib_runtime_mt(${output_target})
# --------------

# COPY PROFILER ----
if (RAWRBOX_BUILD_PROFILER)
    add_custom_target(
            copy_tracy ALL COMMAND ${CMAKE_COMMAND} -E copy ${tracy-profiler_SOURCE_DIR}/tracy-profiler${CMAKE_EXECUTABLE_SUFFIX} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            COMMENT "Copying tracy profiler into binary directory")

    add_dependencies(${output_target} copy_tracy)
endif ()
# --------------

# TEST ----
include(../cmake/catch2.cmake)
# --------------
