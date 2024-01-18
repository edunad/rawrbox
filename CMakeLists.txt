cmake_minimum_required(VERSION 3.16.3 FATAL_ERROR)

# SETUP UTILS
include("cmake/rawrbox_utils.cmake")
# -----

# SETUP CPM -----
include("cmake/cpm.cmake")
CPMUsePackageLock(package-lock.cmake)
# ---------------

# Versioning
set(RAWRBOX_VERSION_MAJOR 2 CACHE STRING "major version" FORCE)
set(RAWRBOX_VERSION_MINOR 0 CACHE STRING "minor version" FORCE)
set(RAWRBOX_VERSION ${RAWRBOX_VERSION_MAJOR}.${RAWRBOX_VERSION_MINOR} CACHE STRING "version" FORCE)

project("RAWRBOX" VERSION ${RAWRBOX_VERSION} DESCRIPTION "Game engine - By edunad" LANGUAGES C CXX)
# ---

# Fix cmake paths
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(PREPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
list(PREPEND CONAN_GENERATORS_FOLDER ${CMAKE_BINARY_DIR})
# ---

# OPTIONS ---
option(RAWRBOX_USE_WAYLAND "Use Wayland for linux" OFF)

option(RAWRBOX_BUILD_TESTING "Build tests" ON)
option(RAWRBOX_BUILD_SAMPLES "Build samples" ON)

option(RAWRBOX_BUILD_RAWRBOX_UI "Build RAWRBOX.UI" OFF)
option(RAWRBOX_BUILD_RAWRBOX_RESOURCES "Build RAWRBOX.RESOURCES" OFF)
option(RAWRBOX_BUILD_RAWRBOX_3D_PHYSICS "Build 3D physics support" OFF)
option(RAWRBOX_BUILD_RAWRBOX_2D_PHYSICS "Build 2D physics support" OFF)
option(RAWRBOX_BUILD_RAWRBOX_BASS "Build bass & add sound loading support" OFF)
option(RAWRBOX_BUILD_RAWRBOX_ASSIMP "Build assimp & add model loading support" OFF)
option(RAWRBOX_BUILD_RAWRBOX_WEBM "Build libwebm & add video loading support" OFF)
option(RAWRBOX_BUILD_RAWRBOX_NETWORK "Build network support" OFF)
option(RAWRBOX_BUILD_RAWRBOX_SCRIPTING "Build lua & mod support" OFF)
option(RAWRBOX_BUILD_RAWRBOX_SCRIPTING_USE_LUAJIT "Enable scripting LuaJIT support (CURRENTLY BROKEN)" OFF)
option(RAWRBOX_BUILD_MSVC_MULTITHREADED_RUNTIME "Build with msvc multithreading" OFF)
option(RAWRBOX_BUILD_QHULL "Include QHull on utils" OFF)

# Supports -----
option(RAWRBOX_DISABLE_SUPPORT_DX12   "Disable dx12 support" OFF)
option(RAWRBOX_DISABLE_SUPPORT_VULKAN "Disable vulkan support" OFF)
# ---------------

option(RAWRBOX_COPY_SHADER_INCLUDES "Copy rawrbox shaders includes" OFF)

option(RAWRBOX_DEV_MODE "Builds all modules, used for developing rawrbox" OFF)
# -----

# Uncomment for package.lock generation
if(RAWRBOX_DEV_MODE)
    message(WARNING "RAWRBOX Dev mode enabled!")

    set(RAWRBOX_BUILD_RAWRBOX_UI ON)
    set(RAWRBOX_BUILD_RAWRBOX_RESOURCES ON)
    #set(RAWRBOX_BUILD_RAWRBOX_3D_PHYSICS ON)
    set(RAWRBOX_BUILD_RAWRBOX_2D_PHYSICS ON)
    set(RAWRBOX_BUILD_RAWRBOX_BASS ON)
    set(RAWRBOX_BUILD_RAWRBOX_ASSIMP ON)
    #set(RAWRBOX_BUILD_RAWRBOX_WEBM ON)
    #set(RAWRBOX_BUILD_RAWRBOX_NETWORK ON)
    #set(RAWRBOX_BUILD_RAWRBOX_SCRIPTING ON)
    #set(RAWRBOX_BUILD_QHULL ON)
endif()

# OTHER SETTINGS ----
set(RAWRBOX_OUTPUT_BIN "bin" CACHE STRING "the target directory of where the output of the build will be stored for use by artifacts")
set(RAWRBOX_CONTENT_FOLDER "assets" CACHE STRING "the assets folder (aka where models, textures, etc are)")
set(RAWRBOX_SHADERS_FOLDER "shaders" CACHE STRING "the shaders folder")
set(RAWRBOX_LIBRARY_TYPE "STATIC" CACHE STRING "Linking type for library")
# ----------------

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSPDLOG_FMT_EXTERNAL -DNOMINMAX")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}")

set(CPM_SOURCE_CACHE "${PROJECT_SOURCE_DIR}/.cache/cpm")

set(BUILD_SHARED_LIBS OFF)

# Apple needs this flag to compile mixed objective/c++
if(APPLE AND NOT XCODE)
    set(CMAKE_CXX_FLAGS "-ObjC++")
endif()

# Linux and Wayland support
if(NOT WIN32)
    set(RAWRBOX_EXTRA_LIBS pthread GL X11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")

    if(RAWRBOX_USE_WAYLAND)
        list(APPEND RAWRBOX_EXTRA_LIBS wayland-egl)
        set(GLFW_BUILD_WAYLAND ON CACHE INTERNAL "")
    endif()
else()
    # Ignore warnings about missing pdb
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4099")
    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /ignore:4099")

    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")
endif()

set(CMAKE_SKIP_RPATH TRUE)
set(CMAKE_SKIP_BUILD_RPATH TRUE)
# ---------------------------------

## CHECKS ---
if(RAWRBOX_BUILD_SAMPLES AND NOT RAWRBOX_BUILD_RAWRBOX_RESOURCES)
    message(WARNING "Samples require RAWRBOX.RESOURCES to be enabled, enabling...")
    set(RAWRBOX_BUILD_RAWRBOX_RESOURCES ON)
endif()

if(RAWRBOX_BUILD_RAWRBOX_UI AND NOT RAWRBOX_BUILD_RAWRBOX_RESOURCES)
    message(WARNING "RAWRBOX.UI requires RAWRBOX.RESOURCES to be enabled, enabling...")
    set(RAWRBOX_BUILD_RAWRBOX_RESOURCES ON)
endif()

if(NOT RAWRBOX_BUILD_MSVC_MULTITHREADED_RUNTIME AND RAWRBOX_BUILD_RAWRBOX_3D_PHYSICS)
    set(RAWRBOX_BUILD_MSVC_MULTITHREADED_RUNTIME ON) # Jolt requires msvc multithreaded runtime
    message(WARNING "JoltPhysics is enabled, forcing RAWRBOX_BUILD_MSVC_MULTITHREADED_RUNTIME")
endif()
# -------------------------

# GLOBAL PACKAGES ------
CPMAddPackage("gh:fmtlib/fmt#10.1.1")
if(fmt_ADDED)
    set_lib_runtime_mt(fmt)
endif()

CPMAddPackage("gh:Neargye/magic_enum@0.9.5")
if(magic_enum_ADDED)
    set_lib_runtime_mt(magic_enum)
endif()

if(NOT WIN32 AND RAWRBOX_USE_WAYLAND)
    CPMAddPackage("gl:wayland/weston@12.0.3")
endif()

CPMAddPackage(
    NAME
        nlohmann_json
    VERSION
        3.11.3
    # the git repo is incredibly large, so we download the archived include directory
    URL
        https://github.com/nlohmann/json/releases/download/v3.11.3/include.zip
    URL_HASH
        SHA256=a22461d13119ac5c78f205d3df1db13403e58ce1bb1794edc9313677313f4a9d)

if(nlohmann_json_ADDED)
    add_library(nlohmann_json INTERFACE IMPORTED)
    target_include_directories(nlohmann_json INTERFACE ${nlohmann_json_SOURCE_DIR}/include)
endif()

# ---
# libcurl keeps complaining it can't find zlibstatic, but when assimp is enabled, this works..
if(RAWRBOX_BUILD_RAWRBOX_ASSIMP)
    CPMFindPackage(
        NAME
            ZLIB
        GITHUB_REPOSITORY
            madler/zlib # zlib-ng/zlib-ng
        GIT_TAG
            "v1.3" # "2.1.3"
        OPTIONS
            "ASM686 FALSE"
            "AMD64 FALSE"
            "POSITION_INDEPENDENT_CODE TRUE"
            "ZLIB_BUILD_EXAMPLES OFF"
    )

    if(ZLIB_ADDED)
        set(ZLIB_FOUND 1)
        set(ZLIB_LIBRARIES zlibstatic)
        set(ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})

        add_library(ZLIB::ZLIB ALIAS zlibstatic)
        set_lib_runtime_mt(zlibstatic)

        message(STATUS "==== ZLIB")
        message(STATUS "LIBRARY: ${ZLIB_LIBRARIES}")
        message(STATUS "INCLUDE_DIR: ${ZLIB_INCLUDE_DIR}")
        message(STATUS "BINARY_DIR: ${zlib_BINARY_DIR}")
        message(STATUS "==== ====")

        include_directories(${ZLIB_INCLUDE_DIR})

    endif()
endif()
# --------------------

add_subdirectory("rawrbox.math")
add_subdirectory("rawrbox.utils")
add_subdirectory("rawrbox.engine")

if(RAWRBOX_BUILD_RAWRBOX_RESOURCES)
    message(STATUS "Enabled RAWRBOX.RESOURCES support")
    add_subdirectory("rawrbox.resources")
endif()

if(RAWRBOX_BUILD_RAWRBOX_NETWORK)
   message(STATUS "Enabled RAWRBOX.NETWORK support")
   add_subdirectory("rawrbox.network")
endif()

#if(RAWRBOX_BUILD_RAWRBOX_SCRIPTING)
#    message(STATUS "Enabled RAWRBOX.SCRIPTING support")
#    add_subdirectory("rawrbox.scripting")
#endif()

add_subdirectory("rawrbox.render")

if(RAWRBOX_BUILD_RAWRBOX_WEBM)
    message(STATUS "Enabled RAWRBOX.WEBM support")
    add_subdirectory("rawrbox.webm")
endif()

if(RAWRBOX_BUILD_RAWRBOX_ASSIMP)
    message(STATUS "Enabled RAWRBOX.ASSIMP support")
    add_subdirectory("rawrbox.assimp")
endif()

if(RAWRBOX_BUILD_RAWRBOX_UI)
    message(STATUS "Enabled RAWRBOX.UI support")
    add_subdirectory("rawrbox.ui")
endif()

if(RAWRBOX_BUILD_RAWRBOX_BASS)
    message(STATUS "Enabled RAWRBOX.BASS support")
    add_subdirectory("rawrbox.bass")
endif()

if(RAWRBOX_BUILD_RAWRBOX_3D_PHYSICS)
    message(STATUS "Enabled RAWRBOX.3D_PHYSICS support")
    add_subdirectory("rawrbox.physics_3d")
endif()

if(RAWRBOX_BUILD_RAWRBOX_2D_PHYSICS)
    message(STATUS "Enabled RAWRBOX.2D_PHYSICS support")
    add_subdirectory("rawrbox.physics_2d")
endif()

# Shhhhh windows... shhhh....
# mute_dependencies_warnings("RAWRBOX")
# --------------------

if(RAWRBOX_BUILD_SAMPLES)
    message(STATUS "Enabled SAMPLES building")
    add_subdirectory("samples")
endif()

