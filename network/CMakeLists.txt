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
    OPTIONS
    "BUILD_SHARED_LIBS 0"
    "CPR_ENABLE_SSL 1"
    "CPR_ENABLE_CURL_HTTP_ONLY 1"
    "CURL_ZLIB 1")

if(libcpr_ADDED)
    set_lib_runtime_mt(libcurl)
    set_lib_runtime_mt(cpr)
endif()

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

set_lib_runtime_mt(${output_target})
# --------------

# TEST ----
#include(../cmake/catch2.cmake)
# --------------
