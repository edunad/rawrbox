cmake_minimum_required(VERSION 3.16.3 FATAL_ERROR)

# SETUP UTILS
include("cmake/rawrbox_utils.cmake")
# -----

# SETUP CPM -----
include("cmake/cpm.cmake")
# ---------------

# Versioning
set(RAWRBOX_VERSION_MAJOR 1 CACHE STRING "major version" FORCE)
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
option(BUILD_TESTING "Build tests" ON)
option(RAWRBOX_USE_WAYLAND "Use Wayland for linux" OFF)

option(RAWRBOX_BUILD_SAMPLES "Build samples" ON)

option(RAWRBOX_BUILD_RAWRBOX_UI "Build RAWRBOX.UI" ON)
option(RAWRBOX_BUILD_RAWRBOX_RESOURCES "Build RAWRBOX.RESOURCES" ON)
option(RAWRBOX_BUILD_RAWRBOX_3D_PHYSICS "Build 3D physics support" ON)
option(RAWRBOX_BUILD_RAWRBOX_2D_PHYSICS "Build 2D physics support" ON)
option(RAWRBOX_BUILD_RAWRBOX_BASS "Build bass & add sound loading support" ON)
option(RAWRBOX_BUILD_RAWRBOX_ASSIMP "Build assimp & add model loading support" ON)
option(RAWRBOX_BUILD_RAWRBOX_WEBM "Build libwebm & add video loading support" ON)

option(RAWRBOX_BUILD_RAWRBOX_SCRIPTING "Build lua & mod support" ON)
option(RAWRBOX_BUILD_RAWRBOX_SCRIPTING_USE_LUAJIT "Enable scripting LuaJIT support" ON)

option(RAWRBOX_BUILD_MSVC_MULTITHREADED_RUNTIME "Build with msvc multithreading" ON)

if(RAWRBOX_BUILD_RAWRBOX_3D_PHYSICS)
    if(RAWRBOX_BUILD_RAWRBOX_2D_PHYSICS)
        message(WARNING "Both 3D & 2D Physics are enabled, make sure you either use 2D or 3D")
    endif()

    set(RAWRBOX_BUILD_MSVC_MULTITHREADED_RUNTIME ON) # Jolt requires msvc multithreaded runtime
    message(WARNING "JoltPhysics is enabled, forcing RAWRBOX_BUILD_MSVC_MULTITHREADED_RUNTIME")
endif()

option(RAWRBOX_ENABLE_QHULL "Include QHull on utils" ON)
# -----

# OTHER SETTINGS ----
set(RAWRBOX_OUTPUT_BIN "bin" CACHE STRING "the target directory of where the output of the build will be stored for use by artifacts")
set(RAWRBOX_CONTENT_FOLDER "content" CACHE STRING "the content folder (aka where shaders, textures, etc are)")
set(RAWRBOX_LIBRARY_TYPE "STATIC" CACHE STRING "Linking type for library")
# ----------------

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSPDLOG_FMT_EXTERNAL -DNOMINMAX")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
        set(BGFX_USE_WAYLAND ON CACHE INTERNAL "")
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

# SETUP CCACHE ---
CPMAddPackage("gh:TheLartians/Ccache.cmake@1.2.4")
# --------------

# PACKAGES ------
CPMAddPackage("gh:fmtlib/fmt#10.0.0")
if(fmt_ADDED)
    set_lib_runtime_mt(fmt)
endif()

CPMAddPackage("gh:Neargye/magic_enum@0.9.0")
if(magic_enum_ADDED)
    set_lib_runtime_mt(magic_enum)
endif()

if(NOT WIN32 AND RAWRBOX_USE_WAYLAND)
    CPMAddPackage("gl:wayland/weston@12.0.1")
endif()

CPMAddPackage(
    NAME
    nlohmann_json
    VERSION
    3.11.2
    # the git repo is incredibly large, so we download the archived include directory
    URL
    https://github.com/nlohmann/json/releases/download/v3.11.2/include.zip
    URL_HASH
    SHA256=e5c7a9f49a16814be27e4ed0ee900ecd0092bfb7dbfca65b5a421b774dccaaed)

if(nlohmann_json_ADDED)
    add_library(nlohmann_json INTERFACE IMPORTED)
    target_include_directories(nlohmann_json INTERFACE ${nlohmann_json_SOURCE_DIR}/include)
endif()
# --------------------

add_subdirectory("math")
add_subdirectory("utils")
add_subdirectory("engine")

if(RAWRBOX_BUILD_RAWRBOX_RESOURCES)
    message(STATUS "Enabled RESOURCES support")
    add_subdirectory("resources")
endif()

if(RAWRBOX_BUILD_RAWRBOX_SCRIPTING)
    message(STATUS "Enabled SCRIPTING support")
    add_subdirectory("scripting")
endif()

add_subdirectory("render")

if(RAWRBOX_BUILD_RAWRBOX_ASSIMP)
    message(STATUS "Enabled ASSIMP support")
    add_subdirectory("assimp")
endif()

if(RAWRBOX_BUILD_RAWRBOX_WEBM)
    message(STATUS "Enabled WEBM support")
    add_subdirectory("webm")
endif()

if(RAWRBOX_BUILD_RAWRBOX_UI)
    message(STATUS "Enabled UI support")
    add_subdirectory("ui")
endif()

if(RAWRBOX_BUILD_RAWRBOX_BASS)
    message(STATUS "Enabled BASS support")
    add_subdirectory("bass")
endif()

if(RAWRBOX_BUILD_RAWRBOX_3D_PHYSICS)
    message(STATUS "Enabled 3D PHYSICS support")
    add_subdirectory("physics_3d")
endif()

if(RAWRBOX_BUILD_RAWRBOX_2D_PHYSICS)
    message(STATUS "Enabled 2D PHYSICS support")
    add_subdirectory("physics_2d")
endif()

if(RAWRBOX_BUILD_SAMPLES)
    if(NOT RAWRBOX_BUILD_RAWRBOX_RESOURCES)
        message(WARNING "Samples require RAWRBOX.RESOURCES to be enabled")
    else()
        message(STATUS "Enabled SAMPLES building")
        add_subdirectory("samples")
    endif()
endif()
