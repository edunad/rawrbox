# Project setup
project("RAWRBOX.ASSIMP" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - ASSIMP lib" LANGUAGES CXX)
set(output_target RAWRBOX.ASSIMP)

# LIBS ---
CPMAddPackage(
    NAME
    assimp
    VERSION
    5.2.5
    GITHUB_REPOSITORY
    assimp/assimp
    OPTIONS
    "BUILD_SHARED_LIBS OFF"
    "ASSIMP_NO_EXPORT ON"
    "ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF"
    "ASSIMP_INSTALL ON"
    "ASSIMP_BUILD_ASSIMP_TOOLS OFF"
    "ASSIMP_BUILD_TESTS OFF"
    "ASSIMP_BUILD_SAMPLES OFF"
    "ASSIMP_BUILD_DOCS OFF"
    "USE_STATIC_CRT ON"
    "ASSIMP_BUILD_ASSIMP_VIEW OFF"
    "ASSIMP_BUILD_ZLIB TRUE"
    "ASSIMP_WARNINGS_AS_ERRORS OFF"
    "ASSIMP_INSTALL_PDB OFF"
    "ASSIMP_BUILD_DRACO OFF"
    "ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF"
    "ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF"
    # IMPORT SUPPORTS
    "ASSIMP_BUILD_OBJ_IMPORTER ON"
    "ASSIMP_BUILD_FBX_IMPORTER ON"
    "ASSIMP_BUILD_DAE_IMPORTER ON"
    "ASSIMP_BUILD_STL_IMPORTER ON"
    "ASSIMP_BUILD_GLTF_IMPORTER ON"
    "ASSIMP_BUILD_3DS_IMPORTER ON"
    # "ASSIMP_BUILD_MDL_IMPORTER ON"
    "ASSIMP_BUILD_MD5_IMPORTER ON")
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
