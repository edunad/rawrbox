# Project setup
project(
    "RAWRBOX.RESOURCES"
    VERSION ${RAWRBOX_VERSION}
    DESCRIPTION "RawrBox - Resources manager lib"
    LANGUAGES CXX)
set(output_target RAWRBOX.RESOURCES)

# LIBS ---
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
# ----

# LIB ----
file(GLOB_RECURSE RAWRBOX_RESOURCES_IMPORTS "src/*.cpp" "include/*.hpp")
add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_RESOURCES_IMPORTS})

target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_compile_definitions(
    ${output_target}
    PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX
    INTERFACE RAWRBOX_RESOURCES)
target_link_libraries(${output_target} PUBLIC RAWRBOX.UTILS fmt nlohmann_json)

set_lib_runtime_mt(${output_target})
# --------------

# TEST ----
include(../cmake/catch2.cmake)
# --------------
