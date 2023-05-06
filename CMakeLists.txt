cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

include(FetchContent)
include("cmake/bgfx_utils.cmake")

# Versioning
set(RAWRBOX_VERSION_MAJOR 1 CACHE STRING "major version" FORCE)
set(RAWRBOX_VERSION_MINOR 0 CACHE STRING "minor version" FORCE)
set(RAWRBOX_VERSION ${RAWRBOX_VERSION_MAJOR}.${RAWRBOX_VERSION_MINOR} CACHE STRING "version" FORCE)

project ("RAWRBOX" VERSION ${RAWRBOX_VERSION} DESCRIPTION "Game engine - By edunad" LANGUAGES CXX)
# ---

# Fix cmake paths
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_HOME_DIRECTORY}/cmake")
list(PREPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
list(PREPEND CONAN_GENERATORS_FOLDER ${CMAKE_BINARY_DIR})
# ---

# OPTIONS ---
option(RAWRBOX_USE_WAYLAND "Use Wayland for linux" OFF)
option(RAWRBOX_BUILD_SAMPLES "Build samples" ON)
option(RAWRBOX_BUILD_TESTING "Build tests" ON)
option(RAWRBOX_ENABLE_QHULL "Include QHull on utils" ON)
option(RAWRBOX_ENABLE_ASSIMP_SUPPORT "Build assimp & add model loading support" ON)
option(RAWRBOX_ENABLE_BASS_SUPPORT "Build bass & add sound loading support" ON)
option(RAWRBOX_ENABLE_DEBUG_SUPPORT "Build debug lib & add gizmo support" ON)
# -----

# OTHER SETTINGS ----
set(RAWRBOX_OUTPUT_BIN "bin" CACHE STRING "the target directory of where the output of the build will be stored for use by artifacts")
set(RAWRBOX_CONTENT_FOLDER "content" CACHE STRING "the content folde (aka where shaders, textures, etc are)")
set(RAWRBOX_LIBRARY_TYPE "SHARED" CACHE STRING "Linking type for library" )

set_property(CACHE RAWRBOX_LIBRARY_TYPE PROPERTY STRINGS STATIC SHARED) # TODO
# ----------------

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSPDLOG_FMT_EXTERNAL -DNOMINMAX")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Apple needs this flag to compile mixed objective/c++
if(APPLE AND NOT XCODE)
	set(CMAKE_CXX_FLAGS "-ObjC++")
endif()

# Linux and Wayland support
if(NOT WIN32)
	set(RAWRBOX_EXTRA_LIBS pthread GL X11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")

	if(RAWRBOX_USE_WAYLAND)
		LIST(APPEND RAWRBOX_EXTRA_LIBS wayland-egl)

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

# PACKAGES ------
find_package(fmt REQUIRED)
if(NOT WIN32 AND RAWRBOX_USE_WAYLAND)
	find_package(wayland REQUIRED)
endif()
#--------------------

# libraries 3th party
set(GLFW_BUILD_DOCS     OFF CACHE INTERNAL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE INTERNAL "" FORCE)
set(GLFW_INSTALL        OFF CACHE INTERNAL "" FORCE)
set(GLFW_USE_OSMESA     OFF CACHE INTERNAL "" FORCE)
set(GLFW_VULKAN_STATIC  OFF CACHE INTERNAL "" FORCE)

set(BGFX_BUILD_TOOLS ON CACHE INTERNAL "" FORCE)
set(BGFX_BUILD_TOOLS_SHADER ON CACHE INTERNAL "" FORCE)
set(BGFX_BUILD_TOOLS_GEOMETRY OFF CACHE INTERNAL "" FORCE)
set(BGFX_BUILD_TOOLS_TEXTURE OFF CACHE INTERNAL "" FORCE)
set(BGFX_BUILD_TOOLS_BIN2C OFF CACHE INTERNAL "" FORCE)

set(BGFX_AMALGAMATED ON CACHE INTERNAL "" FORCE)
set(BX_AMALGAMATED ON CACHE INTERNAL "" FORCE)

set(BGFX_BUILD_EXAMPLES OFF CACHE INTERNAL "" FORCE)
set(BGFX_USE_OVR OFF CACHE INTERNAL "" FORCE)
set(BGFX_INSTALL_EXAMPLES OFF CACHE INTERNAL "" FORCE)

set(ASSIMP_NO_EXPORT ON CACHE BOOL "")
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "")
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "")
set(ASSIMP_BUILD_DOCS OFF CACHE BOOL "")
set(ASSIMP_BUILD_ASSIMP_VIEW OFF CACHE BOOL "")
set(ASSIMP_BUILD_DRACO ON CACHE BOOL "")
set(ASSIMP_INSTALL_PDB OFF CACHE BOOL "")

set(CATCH_INSTALL_DOCS OFF CACHE BOOL "")
set(CATCH_INSTALL_EXTRAS ON CACHE BOOL "")
# ---------------------------------

add_subdirectory ("math")
add_subdirectory ("utils")
add_subdirectory ("render")
add_subdirectory ("engine")

if(RAWRBOX_ENABLE_DEBUG_SUPPORT)
	add_subdirectory ("debug")
endif()

if(RAWRBOX_ENABLE_BASS_SUPPORT)
	add_subdirectory ("bass")
endif()
#add_subdirectory ("ui")

if(RAWRBOX_BUILD_SAMPLES)
	add_subdirectory ("samples")
endif()
