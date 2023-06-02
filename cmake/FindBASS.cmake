# BY edunad / failcake
# https://github.com/edunad/cmake-utils

include(FindPackageHandleStandardArgs)

if(BASS_VERSION EQUAL 0)
    message(FATAL_ERROR "No BASS version selected, set `BASS_VERSION`")
else()
	message("Selected BASS Version: ${BASS_VERSION}")
endif()

set(BASS_URL "http://uk.un4seen.com/files/bass${BASS_VERSION}")
set(BASS_FX_URL "http://www.un4seen.com/files/z/0/bass_fx${BASS_VERSION}")
set(BASS_STATIC_MULTITHREADED ON)

set(BASS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/bass${BASS_VERSION})
set(BASS_FX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/bass_fx${BASS_VERSION})
# -------------------------------------------

if (NOT BASSSDK_FOUND AND NOT TARGET BASS::BASS)
	# Download and setup ------------------
	if (WIN32)
		message(STATUS "Setting bass to WINDOWS")
		set(BASS_URL ${BASS_URL}.zip)
		set(BASS_FX_URL ${BASS_FX_URL}.zip)
		set(BASS_ARCHIVE ${CMAKE_CURRENT_SOURCE_DIR}/deps/bass${BASS_VERSION}.zip)
		set(BASS_FX_ARCHIVE ${CMAKE_CURRENT_SOURCE_DIR}/deps/bass_fx${BASS_VERSION}.zip)
	endif()

	if(UNIX AND NOT APPLE)
		message(STATUS "Setting bass to LINUX")
		set(BASS_URL ${BASS_URL}-linux.zip)
		set(BASS_FX_URL ${BASS_FX_URL}-linux.zip)
		set(BASS_ARCHIVE ${CMAKE_CURRENT_SOURCE_DIR}/deps/bass${BASS_VERSION}-linux.zip)
		set(BASS_FX_ARCHIVE ${CMAKE_CURRENT_SOURCE_DIR}/deps/bass_fx${BASS_VERSION}-linux.zip)
	endif()

	if (APPLE)
		message(STATUS "Setting bass to APPLE")
		set(BASS_URL ${BASS_URL}-osx.zip)
		set(BASS_FX_URL ${BASS_FX_URL}-osx.zip)
		set(BASS_ARCHIVE ${CMAKE_CURRENT_SOURCE_DIR}/deps/bass${BASS_VERSION}-osx.zip)
		set(BASS_FX_ARCHIVE ${CMAKE_CURRENT_SOURCE_DIR}/deps/bass_fx${BASS_VERSION}-osx.zip)
	endif ()

	if(NOT EXISTS ${BASS_ARCHIVE})
		message(STATUS "Bass-CMake: Downloading Bass ${BASS_URL}")
		file(DOWNLOAD ${BASS_URL} ${BASS_ARCHIVE} SHOW_PROGRESS)
	endif()

	if(NOT EXISTS ${BASS_FX_ARCHIVE})
		message(STATUS "Bass-CMake: Downloading Bass ${BASS_FX_URL}")
		file(DOWNLOAD ${BASS_FX_URL} ${BASS_FX_ARCHIVE} SHOW_PROGRESS)
	endif()


	if(NOT EXISTS ${BASS_DIR})
		file(MAKE_DIRECTORY ${BASS_DIR})
		message(STATUS "Bass-CMake: Extracting ${BASS_ARCHIVE}")
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xzf ${BASS_ARCHIVE}
			WORKING_DIRECTORY ${BASS_DIR}
		)
	endif()

	if(NOT EXISTS ${BASS_FX_DIR})
		file(MAKE_DIRECTORY ${BASS_FX_DIR})
		message(STATUS "Bass-CMake: Extracting ${BASS_FX_ARCHIVE}")
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xzf ${BASS_FX_ARCHIVE}
			WORKING_DIRECTORY ${BASS_FX_DIR}
		)
	endif()
	# ---------------------------------------------------
	# ---------------------------------------------------

	if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
		if (APPLE)
			set(BASS_LIB_PATH "")
			set(BASS_LIB_FX_PATH "${BASS_LIB_PATH}") # Same path as bass

			set(BASS_BINARY "${BASS_DIR}/libbass.dylib")
			set(BASS_BINARY_FX "${BASS_FX_DIR}/libbass_fx.dylib")
		elseif(WIN32)
			set(BASS_LIB_PATH "c")
			set(BASS_LIB_FX_PATH "C")

			set(BASS_BINARY "${BASS_DIR}/bass.dll")
			set(BASS_BINARY_FX "${BASS_FX_DIR}/bass_fx.dll")
		else()
			set(BASS_LIB_PATH "libs/x86")
			set(BASS_LIB_FX_PATH "${BASS_LIB_PATH}") # Same path as bass

			set(BASS_BINARY "${BASS_DIR}/${BASS_LIB_PATH}/libbass.so")
			set(BASS_BINARY_FX "${BASS_FX_DIR}/${BASS_LIB_FX_PATH}/libbass_fx.so")
		endif()

		MESSAGE("32 bit bass")
	else ()
		if (APPLE)
			set(BASS_LIB_PATH "")
			set(BASS_LIB_FX_PATH "${BASS_LIB_PATH}") # Same path as bass

			set(BASS_BINARY "${BASS_DIR}/libbass.dylib")
			set(BASS_BINARY_FX "${BASS_FX_DIR}/libbass_fx.dylib")
		elseif(WIN32)
			set(BASS_LIB_PATH "c/x64")
			set(BASS_LIB_FX_PATH "C/x64") # shrug

			set(BASS_BINARY "${BASS_DIR}/x64/bass.dll")
			set(BASS_BINARY_FX "${BASS_FX_DIR}/x64/bass_fx.dll")
		else()
			set(BASS_LIB_PATH "libs/x86_64")
			set(BASS_LIB_FX_PATH "${BASS_LIB_PATH}") # Same path as bass

			set(BASS_BINARY "${BASS_DIR}/${BASS_LIB_PATH}/libbass.so")
			set(BASS_BINARY_FX "${BASS_FX_DIR}/${BASS_LIB_FX_PATH}/libbass_fx.so")
		endif()

		MESSAGE("64 bit bass")
	endif()

	find_library(
		BASS_LIBRARY
		NAMES
		bass
		PATHS
		"${BASS_DIR}/${BASS_LIB_PATH}"
		NO_DEFAULT_PATH
	)

	find_library(
		BASS_LIBRARY_FX
		NAMES
		bass_fx
		PATHS
		"${BASS_FX_DIR}/${BASS_LIB_PATH}/"
		NO_DEFAULT_PATH
	)

	find_path(
		BASS_LIBRARY_INCLUDE_DIR
		bass.h
		${BASS_DIR}/
		${BASS_DIR}/c/
		DOC "Include path for Bass"
	)

	find_path(
		BASS_LIBRARY_FX_INCLUDE_DIR
		bass_fx.h
		${BASS_FX_DIR}/
		${BASS_FX_DIR}/C/
		DOC "Include path for Bass FX"
	)

	add_library(BASS::BASS SHARED IMPORTED)
	add_library(BASS::FX SHARED IMPORTED)

	set_target_properties(
		BASS::BASS
		PROPERTIES
		IMPORTED_NO_SONAME TRUE
		IMPORTED_IMPLIB "${BASS_LIBRARY}"
		IMPORTED_LOCATION "${BASS_BINARY}"
		INTERFACE_INCLUDE_DIRECTORIES "${BASS_LIBRARY_INCLUDE_DIR}"
	)

	set_target_properties(
		BASS::FX
		PROPERTIES
		IMPORTED_NO_SONAME TRUE
		IMPORTED_IMPLIB "${BASS_LIBRARY_FX}"
		IMPORTED_LOCATION "${BASS_BINARY_FX}"
		INTERFACE_INCLUDE_DIRECTORIES "${BASS_LIBRARY_FX_INCLUDE_DIR}"
	)

	if(BASS_STATIC_MULTITHREADED)
		if (("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows" OR "${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore") AND NOT MINGW)
			set_target_properties( BASS::BASS PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
			set_target_properties( BASS::FX PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
		endif()
	endif()

	MESSAGE("BASS library: ${BASS_LIBRARY}")
	MESSAGE("BASS FX library: ${BASS_LIBRARY_FX}")
	MESSAGE("BASS includes: ${BASS_LIBRARY_INCLUDE_DIR}")
	MESSAGE("BASS FX includes: ${BASS_LIBRARY_FX_INCLUDE_DIR}")

	MESSAGE("BASS dlls: ${BASS_BINARY}")
	MESSAGE("BASS FX dlls: ${BASS_BINARY_FX}")

	find_package_handle_standard_args(BASS REQUIRED_VARS BASS_LIBRARY BASS_LIBRARY_FX BASS_LIBRARY_INCLUDE_DIR BASS_LIBRARY_FX_INCLUDE_DIR)
endif()

# MIT License
# Copyright (c) 2023 FailCake

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
