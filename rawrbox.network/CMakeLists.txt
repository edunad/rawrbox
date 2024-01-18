# Project setup
project("RAWRBOX.NETWORK" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - Network lib" LANGUAGES CXX)
set(output_target RAWRBOX.NETWORK)
# --------------

# LIBS ---
CPMAddPackage(
    NAME
        libcpr
    GIT_TAG
        1.10.4
    GITHUB_REPOSITORY
        libcpr/cpr
        #PATCH_COMMAND git apply --reject --whitespace=fix ${CMAKE_CURRENT_SOURCE_DIR}/patch/cpr.patch
    OPTIONS
        "BUILD_SHARED_LIBS OFF"
        "CPR_ENABLE_CURL_HTTP_ONLY ON"
        "CPR_ENABLE_SSL ON"
        "CURL_ZLIB ON")

if(libcpr_ADDED)
    if(NOT WIN32)
        find_package(OpenSSL REQUIRED)

        if(OpenSSL_FOUND)
            if(${OPENSSL_VERSION} VERSION_GREATER_EQUAL "3.0.0")
                add_custom_target(
                    copy_openssl_libssl ALL COMMAND ${CMAKE_COMMAND} -E copy ${OPENSSL_SSL_LIBRARY}
                                            ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/libssl.so.3
                    COMMENT "Copying OpenSSL DLL into binary directory")

                add_custom_target(
                    copy_openssl_libcrypto ALL COMMAND ${CMAKE_COMMAND} -E copy ${OPENSSL_CRYPTO_LIBRARY}
                                            ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/libcrypto.so.3
                    COMMENT "Copying OpenSSL DLL into binary directory")

                set(OPENSSL_COPY_TARGETS copy_openssl_libssl copy_openssl_libcrypto)
                message(STATUS "OpenSSL 3.0 detected, copy targets ${OPENSSL_COPY_TARGETS}")
            else()
                add_custom_target(
                    copy_openssl ALL COMMAND ${CMAKE_COMMAND} -E copy ${OPENSSL_LIBRARIES}
                                            ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}
                    COMMENT "Copying OpenSSL DLL into binary directory")

                set(OPENSSL_COPY_TARGETS copy_openssl)
                message(STATUS "OpenSSL < 3.0 detected, copy targets ${OPENSSL_COPY_TARGETS}")
            endif()
        endif()
    endif()

    set_lib_runtime_mt(libcurl)
    set_lib_runtime_mt(cpr)
endif()
# --------------

# Grab source files
file(GLOB_RECURSE RAWRBOX_NETWORK_IMPORTS "src/*.cpp" "include/*.hpp")
if(RAWRBOX_BUILD_RAWRBOX_SCRIPTING)
    list(APPEND RAWRBOX_EXTRA_LIBS RAWRBOX.SCRIPTING)
else()
    remove_folder("${RAWRBOX_NETWORK_IMPORTS}" "/scripting/")
    set(RAWRBOX_NETWORK_IMPORTS "${CLEAN_SOURCES}")
endif()
# ----

# Project setup
add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_NETWORK_IMPORTS})

if(RAWRBOX_BUILD_RAWRBOX_SCRIPTING)
    copy_lua_libs(TARGET ${output_target})
endif()
target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_NETWORK)
target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC cpr::cpr ${RAWRBOX_EXTRA_LIBS})
if(NOT WIN32)
    add_dependencies(${output_target} ${OPENSSL_COPY_TARGETS})
endif()


set_lib_runtime_mt(${output_target})
# --------------

# TEST ----
#include(../cmake/catch2.cmake)
# --------------
