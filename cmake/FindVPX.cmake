# BY edunad / failcake https://github.com/edunad/cmake-utils

include(FindPackageHandleStandardArgs)

if(NOT DEFINED VPX_VERSION)
    message(FATAL_ERROR "No VPX version selected, set `VPX_VERSION`")
else()
    message("Selected VPX Version: ${VPX_VERSION}")
endif()

if(NOT DEFINED VPX_MSC_VERSION)
    message(STATUS "No MSC version selected, set `VPX_MSC_VERSION` to 17")
    set(VPX_MSC_VERSION 17)
else()
    message("Selected MSC VPX Version: ${VPX_MSC_VERSION}")
endif()

set(VPX_WIN_URL
    "https://github.com/ShiftMediaProject/libvpx/releases/download/${VPX_VERSION}/libvpx_${VPX_VERSION}_msvc${VPX_MSC_VERSION}.zip")
set(VPX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/libvpx_${VPX_VERSION}_msvc${VPX_MSC_VERSION})
# -------------------------------------------

if(NOT VPXSDK_FOUND AND NOT TARGET VPX::VPX)
    # Download and setup ------------------
    if(WIN32)
        message(STATUS "VPX-CMake: Setting vpx to WINDOWS")
        set(VPX_URL ${VPX_WIN_URL})
        set(VPX_ARCHIVE ${CMAKE_CURRENT_SOURCE_DIR}/deps/libvpx_${VPX_VERSION}_msvc${VPX_MSC_VERSION}.zip)

        if(NOT EXISTS ${VPX_ARCHIVE})
            message(STATUS "VPX-CMake: Downloading VPX ${VPX_WIN_URL}")
            file(DOWNLOAD ${VPX_WIN_URL} ${VPX_ARCHIVE} SHOW_PROGRESS)
        endif()

        if(NOT EXISTS ${VPX_DIR})
            file(MAKE_DIRECTORY ${VPX_DIR})
            message(STATUS "VPX-CMake: Extracting ${VPX_ARCHIVE}")
            execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${VPX_ARCHIVE} WORKING_DIRECTORY ${VPX_DIR})
        endif()
    endif()

    # ---------------------------------------------------
    # ---------------------------------------------------

    if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4") # 32 bits
        if(APPLE)
            set(VPX_LIB_PATH "UNKNOWN")
            set(VPX_BINARY "UNKNOWN")
        elseif(WIN32)
            set(VPX_LIB_PATH "${VPX_DIR}/lib/x86")
            set(VPX_BINARY "${VPX_DIR}/bin/x86/vpx.dll")
            set(VPX_LIBRARY_INCLUDE_DIR "${VPX_DIR}/include/")
        else()
            list(APPEND VPX_LIB_PATH "/usr/local/lib/")
            list(APPEND VPX_LIB_PATH "/usr/lib/")
            FIND_PATH(VPX_LIBRARY_INCLUDE_DIR NAMES vpx/vpx_encoder.h PATHS /usr/local/include/ /usr/include/)
        endif()

        message(STATUS "VPX-CMake: 32 bit")
    else()
        if(APPLE)
            set(VPX_LIB_PATH "UNKNOWN")
            set(VPX_BINARY "UNKNOWN")
        elseif(WIN32)
            set(VPX_LIB_PATH "${VPX_DIR}/lib/x64")
            set(VPX_BINARY "${VPX_DIR}/bin/x64/vpx.dll")
            set(VPX_LIBRARY_INCLUDE_DIR "${VPX_DIR}/include/")
        else()
            list(APPEND VPX_LIB_PATH "/usr/lib/x86_64-linux-gnu/")
            list(APPEND VPX_LIB_PATH "/usr/local/lib64/")
            list(APPEND VPX_LIB_PATH "/usr/lib64/")
            list(APPEND VPX_LIB_PATH "/usr/local/lib/")
            list(APPEND VPX_LIB_PATH "/usr/lib/")

            FIND_PATH(VPX_LIBRARY_INCLUDE_DIR NAMES vpx/vpx_encoder.h PATHS /usr/local/include/ /usr/include/)
            set(VPX_BINARY "UNKNOWN")
        endif()

        message(STATUS "VPX-CMake: 64 bit")
    endif()

    find_library(VPX_LIBRARY NAMES vpx PATHS ${VPX_LIB_PATH} NO_DEFAULT_PATH)

    add_library(VPX::VPX SHARED IMPORTED)
    set_target_properties(VPX::VPX PROPERTIES IMPORTED_NO_SONAME TRUE IMPORTED_IMPLIB "${VPX_LIBRARY}" IMPORTED_LOCATION "${VPX_LIBRARY}"
                                              INTERFACE_INCLUDE_DIRECTORIES "${VPX_LIBRARY_INCLUDE_DIR}")

    if(VPX_STATIC_MULTITHREADED)
        if(("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows" OR "${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore") AND NOT MINGW)
            set_target_properties(VPX::VPX PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        endif()
    endif()

    message(STATUS "VPX library: ${VPX_LIBRARY}")
    message(STATUS "VPX includes: ${VPX_LIBRARY_INCLUDE_DIR}")
    message(STATUS "VPX dlls: ${VPX_BINARY}")

    find_package_handle_standard_args(VPX REQUIRED_VARS VPX_LIBRARY VPX_LIBRARY_INCLUDE_DIR VPX_BINARY)
endif()

# MIT License Copyright (c) 2023 FailCake

# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
# ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
