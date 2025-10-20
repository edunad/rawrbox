cmake_minimum_required(VERSION 3.16.3 FATAL_ERROR)

if (POLICY CMP0177)
    cmake_policy(SET CMP0177 NEW)
endif ()

# SETUP UTILS
include("cmake/rawrbox_utils.cmake")
# -----

# SETUP CPM -----
include("cmake/cpm.cmake")
# ---------------

# Versioning
set(RAWRBOX_VERSION_MAJOR 2 CACHE STRING "major version" FORCE)
set(RAWRBOX_VERSION_MINOR 4 CACHE STRING "minor version" FORCE)
set(RAWRBOX_VERSION_PATCH 0 CACHE STRING "patch version" FORCE)
set(RAWRBOX_VERSION ${RAWRBOX_VERSION_MAJOR}.${RAWRBOX_VERSION_MINOR}.${RAWRBOX_VERSION_PATCH} CACHE STRING "version" FORCE)

project("RAWRBOX" VERSION ${RAWRBOX_VERSION} DESCRIPTION "Game engine - By edunad" LANGUAGES C CXX)
# ---

# Fix cmake paths
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(PREPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
# ---

# OPTIONS ---
option(RAWRBOX_USE_WAYLAND "Use Wayland for linux" OFF)

# Build -----
option(RAWRBOX_BUILD_TESTING "Build tests" OFF)
option(RAWRBOX_BUILD_SAMPLES "Build samples" OFF)

option(RAWRBOX_BUILD_RAWRBOX_RENDER "Build rawrbox renderer, disable for renderless programs" ON)
option(RAWRBOX_BUILD_RAWRBOX_UI "Build rawrbox UI" OFF)
option(RAWRBOX_BUILD_RAWRBOX_RESOURCES "Build rawrbox resources utils" OFF)
option(RAWRBOX_BUILD_RAWRBOX_3D_PHYSICS "Build 3D physics support" OFF)
option(RAWRBOX_BUILD_RAWRBOX_2D_PHYSICS "Build 2D physics support" OFF)
option(RAWRBOX_BUILD_RAWRBOX_BASS "Build bass & add sound loading support" OFF)
option(RAWRBOX_BUILD_RAWRBOX_GLTF "Build gltf model loading support" OFF)
option(RAWRBOX_BUILD_RAWRBOX_WEBM "Build libwebm & add video loading support" OFF)
option(RAWRBOX_BUILD_RAWRBOX_NETWORK "Build network support" OFF)

option(RAWRBOX_BUILD_RAWRBOX_SCRIPTING "Build lua & mod support" OFF)
option(RAWRBOX_SCRIPTING_UNSAFE "Enable unsafe scripting (io / etc)" OFF)
option(RAWRBOX_SCRIPTING_WORKSHOP_MODDING "Enables workshop utilities (useful for steam workshop / mod.io)" OFF)
option(RAWRBOX_SCRIPTING_EXCEPTION "Enables scripting throwing exceptions instead of catching them" OFF)

option(RAWRBOX_BUILD_RAWRBOX_STEAMWORKS "Build steamworks support" OFF)
option(RAWRBOX_BUILD_RAWRBOX_IMGUI "Build imgui support" OFF)
option(RAWRBOX_BUILD_MSVC_MULTITHREADED_RUNTIME "Build with msvc multithreading" OFF)
option(RAWRBOX_BUILD_QHULL "Include QHull on utils" OFF)
# ---------------

# Supports -----
option(RAWRBOX_DISABLE_SUPPORT_DX12 "Disable dx12 support" OFF)
option(RAWRBOX_DISABLE_SUPPORT_VULKAN "Disable vulkan support" OFF)
# ---------------

# Other -----
option(RAWRBOX_DEV_MODE "Builds all modules, used for developing rawrbox" OFF)
option(RAWRBOX_INTERPROCEDURAL_OPTIMIZATION "Enables IPO" ON)
# ---------------
# -----

if (RAWRBOX_INTERPROCEDURAL_OPTIMIZATION AND NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ARM64") AND NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ARM"))
    message(STATUS "Enabled INTERPROCEDURAL_OPTIMIZATION for release and distribution")

    cmake_policy(SET CMP0069 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipoSupported OUTPUT error)

    if (ipoSupported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DISTRIBUTION TRUE)
    else ()
        message(FATAL_ERROR "IPO / LTO is not supported, please disable RAWRBOX_INTERPROCEDURAL_OPTIMIZATION")
    endif ()
endif ()

if (RAWRBOX_DEV_MODE)
    message(WARNING "RAWRBOX Dev mode enabled!")

    set(RAWRBOX_BUILD_TESTING ON)
    set(RAWRBOX_BUILD_SAMPLES ON)

    set(RAWRBOX_BUILD_RAWRBOX_UI ON)
    set(RAWRBOX_BUILD_RAWRBOX_RESOURCES ON)
    set(RAWRBOX_BUILD_RAWRBOX_GLTF ON)
    set(RAWRBOX_BUILD_RAWRBOX_3D_PHYSICS ON)
    set(RAWRBOX_BUILD_RAWRBOX_2D_PHYSICS ON)
    set(RAWRBOX_BUILD_RAWRBOX_BASS ON)
    set(RAWRBOX_BUILD_RAWRBOX_WEBM ON)
    set(RAWRBOX_BUILD_RAWRBOX_NETWORK ON)

    set(RAWRBOX_BUILD_RAWRBOX_SCRIPTING ON)
    set(RAWRBOX_SCRIPTING_UNSAFE ON)
    set(RAWRBOX_SCRIPTING_WORKSHOP_MODDING ON)

    set(RAWRBOX_BUILD_RAWRBOX_STEAMWORKS ON)
    set(RAWRBOX_BUILD_RAWRBOX_IMGUI ON)
    set(RAWRBOX_BUILD_QHULL ON)

    if (NOT DEFINED STEAMWORKS_APPID)
        message(STATUS "Set STEAMWORKS_APPID to 480 (SpaceWars example game)")
        set(STEAMWORKS_APPID 480) # SpaceWars example game
    endif ()

    set(CPM_SOURCE_CACHE "${PROJECT_SOURCE_DIR}/.cache/cpm")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
endif ()

# OTHER SETTINGS ----
set(RAWRBOX_CONTENT_FOLDER "assets" CACHE STRING "The assets folder (aka where models, textures, etc are)")
set(RAWRBOX_LIBRARY_TYPE "STATIC" CACHE STRING "Linking type for library")
# ----------------

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BUILD_SHARED_LIBS OFF)

# Apple needs this flag to compile mixed objective/c++
if (APPLE AND NOT XCODE)
    set(CMAKE_CXX_FLAGS "-ObjC++")
endif ()

# Linux and Wayland support
if (NOT WIN32)
    set(RAWRBOX_EXTRA_LIBS pthread GL X11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")

    if (RAWRBOX_USE_WAYLAND)
        list(APPEND RAWRBOX_EXTRA_LIBS wayland-egl)
        set(GLFW_BUILD_WAYLAND ON CACHE INTERNAL "")
    endif ()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti -fexceptions") # Enable exceptions and RTTI
else ()
    # Ignore warnings about missing pdb
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4099")
    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /ignore:4099")

    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive- /EHsc /GR") # Enable strict mode, exceptions and RTTI
endif ()

set(CMAKE_SKIP_RPATH TRUE)
set(CMAKE_SKIP_BUILD_RPATH TRUE)
# ---------------------------------

# Patch vs studio paths
if (WIN32 AND NOT VS_DXC_COMPILER_PATH AND NOT VS_DXIL_SIGNER_PATH AND NOT VS_D3D_COMPILER_PATH)
    get_filename_component(CMAKE_DEVKIT_LOCATION ${CMAKE_RC_COMPILER} DIRECTORY)

    set(VS_DXC_COMPILER_PATH "${CMAKE_DEVKIT_LOCATION}/dxcompiler.dll" CACHE INTERNAL "")
    set(VS_DXIL_SIGNER_PATH "${CMAKE_DEVKIT_LOCATION}/dxil.dll" CACHE INTERNAL "")
    set(VS_D3D_COMPILER_PATH "${CMAKE_DEVKIT_LOCATION}/d3dcompiler_47.dll" CACHE INTERNAL "")

    message(STATUS "Patching DirectX Visual Studio paths:\n\tVS_DXC_COMPILER_PATH: ${VS_DXC_COMPILER_PATH}\n\tVS_DXIL_SIGNER_PATH: ${VS_DXIL_SIGNER_PATH}\n\tVS_D3D_COMPILER_PATH: ${VS_D3D_COMPILER_PATH}")
endif ()
# ---------------------

set(RAWRBOX_BUILD_MSVC_MULTITHREADED_RUNTIME OFF)

## CHECKS ---
if (NOT RAWRBOX_BUILD_RAWRBOX_RENDER)
    message(WARNING "----------------------------------------\nRAWRBOX_BUILD_RAWRBOX_RENDER is disabled!\nDisabling UI / GLTF / IMGUI / WEBM and samples..\n----------------------------------------")
    set(RAWRBOX_BUILD_RAWRBOX_UI OFF)
    set(RAWRBOX_BUILD_RAWRBOX_GLTF OFF)
    set(RAWRBOX_BUILD_RAWRBOX_IMGUI OFF)
    set(RAWRBOX_BUILD_RAWRBOX_WEBM OFF)

    set(RAWRBOX_BUILD_SAMPLES OFF)
else ()
    # Samples & UI ---
    if (RAWRBOX_BUILD_SAMPLES AND NOT RAWRBOX_BUILD_RAWRBOX_RESOURCES)
        message(WARNING "Samples require RAWRBOX.RESOURCES to be enabled, enabling...")
        set(RAWRBOX_BUILD_RAWRBOX_RESOURCES ON)
    endif ()

    if (RAWRBOX_BUILD_RAWRBOX_UI AND NOT RAWRBOX_BUILD_RAWRBOX_RESOURCES)
        message(WARNING "RAWRBOX.UI requires RAWRBOX.RESOURCES to be enabled, enabling...")
        set(RAWRBOX_BUILD_RAWRBOX_RESOURCES ON)
    endif ()
    # -----------
endif ()

if (NOT RAWRBOX_BUILD_MSVC_MULTITHREADED_RUNTIME AND RAWRBOX_BUILD_RAWRBOX_3D_PHYSICS)
    set(RAWRBOX_BUILD_MSVC_MULTITHREADED_RUNTIME ON) # Jolt requires msvc multithreaded runtime
    message(WARNING "JoltPhysics is enabled, forcing RAWRBOX_BUILD_MSVC_MULTITHREADED_RUNTIME")
endif ()
# -------------------------

# GLOBAL PACKAGES ------
CPMAddPackage("gh:fmtlib/fmt#12.0.0")
if (fmt_ADDED)
    set_lib_runtime_mt(fmt)
endif ()

CPMAddPackage("gh:Neargye/magic_enum@0.9.7")
if (magic_enum_ADDED)
    set_lib_runtime_mt(magic_enum)
endif ()

if (RAWRBOX_BUILD_RAWRBOX_RENDER)
    if (NOT WIN32 AND RAWRBOX_USE_WAYLAND)
        CPMAddPackage("https://gitlab.freedesktop.org/wayland/weston@12.0.5")
    endif ()
endif ()

CPMAddPackage("gh:stephenberry/glaze@6.0.0")

# ---
if (RAWRBOX_BUILD_RAWRBOX_NETWORK)
    message(WARNING "RAWRBOX.NETWORK is enabled, adding zlib-ng package")

    CPMFindPackage(
            NAME
            zlib
            GITHUB_REPOSITORY
            zlib-ng/zlib-ng
            USE_CACHE
            OFF
            GIT_TAG
            2.2.4
            OPTIONS
            "ZLIB_COMPAT ON"
            "ZLIB_BUILD_EXAMPLES OFF"
            "ZLIB_ENABLE_TESTS OFF"

            "POSITION_INDEPENDENT_CODE ON"
            "WITH_GTEST OFF"
    )

    if (zlib_ADDED)
        include_directories(${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
        set_lib_runtime_mt(zlib)
    endif ()
endif ()
# --------------------

add_subdirectory("rawrbox.math")
add_subdirectory("rawrbox.utils")
add_subdirectory("rawrbox.engine")

if (RAWRBOX_BUILD_RAWRBOX_RENDER)
    message(STATUS "Enabled RAWRBOX.RENDER support")
    add_subdirectory("rawrbox.render")
endif ()

if (RAWRBOX_BUILD_RAWRBOX_SCRIPTING)
    message(STATUS "Enabled RAWRBOX.SCRIPTING support")
    add_subdirectory("rawrbox.scripting")
endif ()

if (RAWRBOX_BUILD_RAWRBOX_RESOURCES)
    message(STATUS "Enabled RAWRBOX.RESOURCES support")
    add_subdirectory("rawrbox.resources")
endif ()

if (RAWRBOX_BUILD_RAWRBOX_NETWORK)
    message(STATUS "Enabled RAWRBOX.NETWORK support")
    add_subdirectory("rawrbox.network")
endif ()

if (RAWRBOX_BUILD_RAWRBOX_WEBM)
    message(STATUS "Enabled RAWRBOX.WEBM support")
    add_subdirectory("rawrbox.webm")
endif ()

if (RAWRBOX_BUILD_RAWRBOX_GLTF)
    message(STATUS "Enabled RAWRBOX.GLTF support")
    add_subdirectory("rawrbox.gltf")
endif ()

if (RAWRBOX_BUILD_RAWRBOX_UI)
    message(STATUS "Enabled RAWRBOX.UI support")
    add_subdirectory("rawrbox.ui")
endif ()

if (RAWRBOX_BUILD_RAWRBOX_BASS)
    message(STATUS "Enabled RAWRBOX.BASS support")
    add_subdirectory("rawrbox.bass")
endif ()

if (RAWRBOX_BUILD_RAWRBOX_2D_PHYSICS)
    message(STATUS "Enabled RAWRBOX.2D_PHYSICS support")
    add_subdirectory("rawrbox.physics_2d")
endif ()

if (RAWRBOX_BUILD_RAWRBOX_STEAMWORKS)
    message(STATUS "Enabled RAWRBOX.STEAMWORKS support")
    add_subdirectory("rawrbox.steamworks")
endif ()

if (RAWRBOX_BUILD_RAWRBOX_IMGUI)
    message(STATUS "Enabled RAWRBOX.IMGUI support")
    add_subdirectory("rawrbox.imgui")
endif ()

# Jolt enables some weird flags, make sure it's the last one
if (RAWRBOX_BUILD_RAWRBOX_3D_PHYSICS)
    message(STATUS "Enabled RAWRBOX.3D_PHYSICS support")
    add_subdirectory("rawrbox.physics_3d")
endif ()
# --------------------

# Shhhhh windows... shhhh....
mute_dependencies_warnings(".*RAWRBOX.*")
# --------------------

if (RAWRBOX_BUILD_SAMPLES)
    message(STATUS "Enabled SAMPLES building")
    add_subdirectory("samples")
endif ()

if (WIN32)
    string(REPLACE "/WX" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif ()
