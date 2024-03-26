# BY edunad / failcake https://github.com/edunad/cmake-utils

include(FindPackageHandleStandardArgs)

if(NOT DEFINED DXC_VERSION) # v1.8.2403.1
    message(FATAL_ERROR "No DXC version selected, set `DXC_VERSION`")
else()
    message("Selected DXC Version: ${DXC_VERSION}")
endif()

if(NOT DEFINED DXC_RELEASE_DATE) # 2024_03_22
    message(FATAL_ERROR "No DXC release date selected, set `DXC_RELEASE_DATE`")
else()
    message("Selected DXC Release Date: ${DXC_RELEASE_DATE}")
endif()

set(DXC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/dxc${DXC_VERSION})
set(DXC_BASE_URL "https://github.com/microsoft/DirectXShaderCompiler/releases/download/${DXC_VERSION}")

if(NOT DXC_FOUND)
    # Download and setup ------------------
    if(WIN32)
        message(STATUS "DXC-CMake: Setting dxc to WINDOWS")
        set(DXC_URL "${DXC_BASE_URL}/dxc_${DXC_RELEASE_DATE}.zip")
        set(DXC_ARCHIVE ${DXC_DIR}.zip)
    endif()

    if(UNIX AND NOT APPLE)
        message(STATUS "DXC-CMake: Setting dxc to LINUX")
        set(DXC_URL "${DXC_BASE_URL}/linux_dxc_${DXC_RELEASE_DATE}.x86_64.tar.gz")
        set(DXC_ARCHIVE ${DXC_DIR}.x86_64.tar.gz)
    endif()
    # -------------------------------------

    # Download ---
    if(NOT EXISTS ${DXC_ARCHIVE})
        message(STATUS "DXC-CMake: Downloading DXC ${DXC_URL}")
        file(DOWNLOAD ${DXC_URL} ${DXC_ARCHIVE} SHOW_PROGRESS)
    endif()

    # Extract -----
    if(NOT EXISTS ${DXC_DIR})
        file(MAKE_DIRECTORY ${DXC_DIR})
        message(STATUS "DXC-CMake: Extracting ${DXC_ARCHIVE}")
        execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${DXC_ARCHIVE} WORKING_DIRECTORY ${DXC_DIR})
    endif()

    # ---------------------------------------------------
    # ---------------------------------------------------
    if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
        if(UNIX AND NOT APPLE)
            message(FATAL_ERROR "x86 linux dxc not supported")
        else()
            set(DXC_BINARY "${DXC_DIR}/bin/x86/dxcompiler.dll")
            set(DXC_BINARY_IL "${DXC_DIR}/bin/x86/dxil.dll")
        endif()

        message(STATUS "DXC-CMake: 32 bit")
    else()
        if(UNIX AND NOT APPLE)
            set(DXC_BINARY "${DXC_DIR}/lib/libdxcompiler.so")
            set(DXC_BINARY_IL "${DXC_DIR}/lib/libdxil.so")
        else()
            set(DXC_BINARY "${DXC_DIR}/bin/x64/dxcompiler.dll")
            set(DXC_BINARY_IL "${DXC_DIR}/bin/x64/dxil.dll")
        endif()

        message(STATUS "DXC-CMake: 64 bit")
    endif()

    message(STATUS "DXC binary: ${DXC_BINARY}")
    message(STATUS "DXCIL binary: ${DXC_BINARY_IL}")

    find_package_handle_standard_args(DXC REQUIRED_VARS DXC_DIR DXC_BINARY DXC_BINARY_IL)
endif()

# MIT License Copyright (c) 2024 FailCake

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
