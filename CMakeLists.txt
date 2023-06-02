cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

# SETUP CPM -----
include("cmake/cpm.cmake")
# ---------------

# Versioning
set(RAWRBOX_VERSION_MAJOR 1 CACHE STRING "major version" FORCE)
set(RAWRBOX_VERSION_MINOR 0 CACHE STRING "minor version" FORCE)
set(RAWRBOX_VERSION ${RAWRBOX_VERSION_MAJOR}.${RAWRBOX_VERSION_MINOR} CACHE STRING "version" FORCE)

project ("RAWRBOX" VERSION ${RAWRBOX_VERSION} DESCRIPTION "Game engine - By edunad" LANGUAGES C CXX)
# ---

# Fix cmake paths
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(PREPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
list(PREPEND CONAN_GENERATORS_FOLDER ${CMAKE_BINARY_DIR})
# ---

# OPTIONS ---
option(RAWRBOX_USE_WAYLAND "Use Wayland for linux" OFF)

option(RAWRBOX_BUILD_SAMPLES "Build samples" ON)
option(RAWRBOX_BUILD_TESTING "Build tests" ON)
option(RAWRBOX_BUILD_UI "Build ui" ON)
option(RAWRBOX_BUILD_RESOURCES_MANAGER "Build resources manager" ON)
option(RAWRBOX_BUILD_DEBUG "Build debug lib & add gizmo support" ON)
option(RAWRBOX_BUILD_PHYSICS "Build physics support" ON)

option(RAWRBOX_ENABLE_QHULL "Include QHull on utils" ON)

option(RAWRBOX_ENABLE_ASSIMP_SUPPORT "Build assimp & add model loading support" ON)
option(RAWRBOX_ENABLE_BASS_SUPPORT "Build bass & add sound loading support" ON)
# -----

# OTHER SETTINGS ----
set(RAWRBOX_OUTPUT_BIN "bin" CACHE STRING "the target directory of where the output of the build will be stored for use by artifacts")
set(RAWRBOX_CONTENT_FOLDER "content" CACHE STRING "the content folder (aka where shaders, textures, etc are)")
set(RAWRBOX_LIBRARY_TYPE "STATIC" CACHE STRING "Linking type for library" )
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

# SETUP CCACHE ---
CPMAddPackage(
  NAME Ccache.cmake
  GITHUB_REPOSITORY TheLartians/Ccache.cmake
  VERSION 1.2.4
)
# --------------

# PACKAGES ------
CPMAddPackage(
	NAME fmt
	GITHUB_REPOSITORY fmtlib/fmt
	GIT_TAG 10.0.0
	OPTIONS
		"FMT_DOC OFF"
		"FMT_TEST OFF"
)

CPMAddPackage(
	NAME magic_enum
	GITHUB_REPOSITORY Neargye/magic_enum
	VERSION 0.9.0
)

if (("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows" OR "${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore") AND NOT MINGW)
    set_property(TARGET fmt PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET magic_enum PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(NOT WIN32 AND RAWRBOX_USE_WAYLAND)
	CPMAddPackage("gl:wayland/weston@12.0.1")
endif()
#--------------------

add_subdirectory ("math")
add_subdirectory ("utils")
add_subdirectory ("engine")

if(RAWRBOX_BUILD_RESOURCES_MANAGER)
	add_subdirectory ("resources")
endif()

add_subdirectory ("render")

if(RAWRBOX_BUILD_UI)
	add_subdirectory ("ui")
endif()

if(RAWRBOX_ENABLE_BASS_SUPPORT)
	add_subdirectory ("bass")
endif()

if(RAWRBOX_BUILD_DEBUG)
	add_subdirectory ("debug")
endif()

if(RAWRBOX_BUILD_PHYSICS)
	add_subdirectory ("physics")
endif()

if(RAWRBOX_BUILD_SAMPLES)
	add_subdirectory ("samples")
endif()
