# By edunad / failcake https://github.com/edunad/cmake-utils
include(FindPackageHandleStandardArgs)

if(NOT EXISTS "${STEAMWORKS_SDKPATH}")
   message(FATAL_ERROR "No Steamworks API path selected, download SDK and set `STEAMWORKS_SDKPATH` to base folder")
endif()

if(NOT DEFINED STEAMWORKS_APPID)
   message(FATAL_ERROR "No Steamworks APPID selected, set `STEAMWORKS_APPID` to the appid")
endif()

if(NOT STEAMWORKS_FOUND)
    FIND_PATH(STEAMWORKS_INCLUDE_DIR steam/steam_api.h
        /usr/include
        /usr/local/include
        ${STEAMWORKS_SDKPATH}/public/
        DOC "Include path for Steamworks"
    )

    # FIND LIB ----
    if(WIN32)
        if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
            set(STEAMWORKS_LIBRARIES "${STEAMWORKS_SDKPATH}/redistributable_bin/steam_api.lib")
            set(STEAMWORKS_LIBRARIES_APPTICKET "${STEAMWORKS_SDKPATH}/public/steam/lib/win32/sdkencryptedappticket.lib")

            set(STEAMWORKS_BINARIES "${STEAMWORKS_SDKPATH}/redistributable_bin/steam_api.dll")
            set(STEAMWORKS_BINARIES_APPTICKET "${STEAMWORKS_SDKPATH}/public/steam/lib/win32/sdkencryptedappticket.dll")

            MESSAGE("32 bit steam")
        else ()
            set(STEAMWORKS_LIBRARIES "${STEAMWORKS_SDKPATH}/redistributable_bin/win64/steam_api64.lib")
            set(STEAMWORKS_LIBRARIES_APPTICKET "${STEAMWORKS_SDKPATH}/public/steam/lib/win64/sdkencryptedappticket64.lib")

            set(STEAMWORKS_BINARIES "${STEAMWORKS_SDKPATH}/redistributable_bin/win64/steam_api64.dll")
            set(STEAMWORKS_BINARIES_APPTICKET "${STEAMWORKS_SDKPATH}/public/steam/lib/win64/sdkencryptedappticket64.dll")

            MESSAGE("64 bit steam")
        endif()
    elseif (APPLE)
        set(STEAMWORKS_LIBRARIES "${STEAMWORKS_SDKPATH}/redistributable_bin/osx/libsteam_api.dylib")
        set(STEAMWORKS_LIBRARIES_APPTICKET "${STEAMWORKS_SDKPATH}/public/steam/lib/osx/libsdkencryptedappticket.dylib")

        set(STEAMWORKS_BINARIES "${STEAMWORKS_LIBRARIES}")
        set(STEAMWORKS_BINARIES_APPTICKET "${STEAMWORKS_LIBRARIES_APPTICKET}")
        MESSAGE("apple steam")
	else()
        if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
            set(STEAMWORKS_LIBRARIES "${STEAMWORKS_SDKPATH}/redistributable_bin/linux32/libsteam_api.so")
            set(STEAMWORKS_LIBRARIES_APPTICKET "${STEAMWORKS_SDKPATH}/public/steam/lib/linux32/libsdkencryptedappticket.so")

            set(STEAMWORKS_BINARIES "${STEAMWORKS_LIBRARIES}")
            set(STEAMWORKS_BINARIES_APPTICKET "${STEAMWORKS_LIBRARIES_APPTICKET}")
            MESSAGE("32 bit steam")
        else()
            set(STEAMWORKS_LIBRARIES "${STEAMWORKS_SDKPATH}/redistributable_bin/linux64/libsteam_api.so")
            set(STEAMWORKS_LIBRARIES_APPTICKET "${STEAMWORKS_SDKPATH}/public/steam/lib/linux64/libsdkencryptedappticket.so")

            set(STEAMWORKS_BINARIES "${STEAMWORKS_LIBRARIES}")
            set(STEAMWORKS_BINARIES_APPTICKET "${STEAMWORKS_LIBRARIES_APPTICKET}")
            MESSAGE("64 bit steam")
        endif()
    endif ()
    # ----------------------

	MESSAGE(STATUS "Steam libs: ${STEAMWORKS_LIBRARIES} ${STEAMWORKS_LIBRARIES_APPTICKET}")
	MESSAGE(STATUS "Steam bins: ${STEAMWORKS_BINARIES} ${STEAMWORKS_BINARIES_APPTICKET}")
	MESSAGE(STATUS "Steam include: ${STEAMWORKS_INCLUDE_DIR}")

	add_library(STEAMWORKS::STEAMWORKS SHARED IMPORTED)
    set_target_properties(STEAMWORKS::STEAMWORKS
        PROPERTIES
        IMPORTED_NO_SONAME
            TRUE
        IMPORTED_IMPLIB
            "${STEAMWORKS_LIBRARIES}"
        IMPORTED_LOCATION
            "${STEAMWORKS_BINARIES}"
        INTERFACE_INCLUDE_DIRECTORIES
            "${STEAMWORKS_INCLUDE_DIR}"
    )

    add_library(STEAMWORKS::APPTICKET SHARED IMPORTED)
    set_target_properties(STEAMWORKS::APPTICKET
        PROPERTIES
        IMPORTED_NO_SONAME
            TRUE
        IMPORTED_IMPLIB
            "${STEAMWORKS_LIBRARIES_APPTICKET}"
        IMPORTED_LOCATION
            "${STEAMWORKS_BINARIES_APPTICKET}"
        INTERFACE_INCLUDE_DIRECTORIES
            "${STEAMWORKS_INCLUDE_DIR}"
    )

    add_custom_target(
        copy_steamworks ALL
        COMMAND ${CMAKE_COMMAND} -E copy ${STEAMWORKS_BINARIES}
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
        COMMENT "Copying steamworks into binary directory")

    add_custom_target(
        copy_steamworks_appticket ALL
        COMMAND ${CMAKE_COMMAND} -E copy ${STEAMWORKS_BINARIES_APPTICKET}
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
        COMMENT "Copying steamworks into binary directory")

    add_custom_target(
        create_steamworks ALL
        COMMAND ${CMAKE_COMMAND} -E echo ${STEAMWORKS_APPID} > ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/steam_appid.txt
        COMMENT "Copying steamworks appid into binary directory")

    add_dependencies(STEAMWORKS::STEAMWORKS copy_steamworks create_steamworks)
    add_dependencies(STEAMWORKS::APPTICKET copy_steamworks_appticket)

    find_package_handle_standard_args(STEAMWORKS REQUIRED_VARS STEAMWORKS_LIBRARIES STEAMWORKS_LIBRARIES_APPTICKET STEAMWORKS_BINARIES STEAMWORKS_BINARIES_APPTICKET STEAMWORKS_INCLUDE_DIR)
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
