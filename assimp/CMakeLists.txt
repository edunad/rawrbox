# Project setup
project("RAWRBOX.ASSIMP" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - ASSIMP lib" LANGUAGES CXX)
set(output_target RAWRBOX.ASSIMP)

# LIBS ---
find_library(ZLIB_FOUND ZLIB QUIET)
if(ZLIB_FOUND)
    message(STATUS "Found zlib, turning off build zlib on assimp")
    set(BUILD_ZLIB 0)
else()
    set(BUILD_ZLIB 1)
endif()

CPMAddPackage(
    NAME
    assimp
    GIT_TAG
    master
    GITHUB_REPOSITORY
    assimp/assimp
    OPTIONS
    "BUILD_SHARED_LIBS 0"
    "ASSIMP_NO_EXPORT 1"
    "ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT 0"
    "ASSIMP_INSTALL 1"
    "ASSIMP_BUILD_ASSIMP_TOOLS 0"
    "ASSIMP_BUILD_TESTS 0"
    "ASSIMP_BUILD_SAMPLES 0"
    "ASSIMP_BUILD_DOCS 0"
    "USE_STATIC_CRT 1"
    "ASSIMP_BUILD_ASSIMP_VIEW 0"
    "ASSIMP_BUILD_ZLIB ${BUILD_ZLIB}"
    "ASSIMP_WARNINGS_AS_ERRORS 0"
    "ASSIMP_INSTALL_PDB 0"
    "ASSIMP_BUILD_DRACO 0"
    "ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT 0"
    "ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT 0"
    # IMPORT SUPPORTS
    "ASSIMP_BUILD_OBJ_IMPORTER 1"
    "ASSIMP_BUILD_FBX_IMPORTER 1"
    "ASSIMP_BUILD_DAE_IMPORTER 1"
    "ASSIMP_BUILD_STL_IMPORTER 1"
    "ASSIMP_BUILD_GLTF_IMPORTER 1"
    "ASSIMP_BUILD_3DS_IMPORTER 1"
    # "ASSIMP_BUILD_MDL_IMPORTER 1"
    "ASSIMP_BUILD_MD5_IMPORTER 1")
# ----

# Grab source files
file(GLOB_RECURSE RAWRBOX_ASSIMP_IMPORTS "src/*.cpp" "include/*.hpp")

if(RAWRBOX_BUILD_RAWRBOX_RESOURCES)
    set(RAWRBOX_EXTRA_LIBS RAWRBOX.RESOURCES)
else()
    remove_folder("${RAWRBOX_ASSIMP_IMPORTS}" "/resources/")
    set(RAWRBOX_ASSIMP_IMPORTS "${CLEAN_SOURCES}")
endif()
# ----

add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_ASSIMP_IMPORTS})
target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_ASSIMP)
target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC RAWRBOX.RENDER assimp::assimp ${RAWRBOX_EXTRA_LIBS})

set_lib_runtime_mt(${output_target})
# --------------

# TEST ---- include(../cmake/catch2.cmake)
# --------------
