# Project setup
project("RAWRBOX.ASSIMP" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - ASSIMP lib" LANGUAGES CXX)
set(output_target RAWRBOX.ASSIMP)

# LIBS ---
if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo" AND RAWRBOX_BUILD_MSVC_MULTITHREADED_RUNTIME)
    set(ASSIMP_BUILD_DRACO OFF) # Draco does not work correctly with RelWithDebInfo
    message(WARNING "RelWithDebInfo is set, turning off ASSIMP_DRACO support")
else()
    set(ASSIMP_BUILD_DRACO ON)
endif()

# SETUP NG-ZLIB
if(zlib_ADDED)
    add_definitions(-DASSIMP_BUILD_NO_OWN_ZLIB)
    set(ZLIB_FOUND TRUE)
    set(ZLIB_LIBRARIES zlib)
endif()
#-----------------

CPMAddPackage(
    NAME
        assimp
    GIT_TAG
        master
    #PATCH_COMMAND
    #    git apply --reject --whitespace=fix ${CMAKE_CURRENT_SOURCE_DIR}/patch/assimp.patch
    GITHUB_REPOSITORY
        edunad/assimp
    OPTIONS
        "BUILD_SHARED_LIBS OFF"
        "ASSIMP_NO_EXPORT ON"

        "ASSIMP_BUILD_ZLIB OFF"
        "ASSIMP_BUILD_ASSIMP_TOOLS OFF"
        "ASSIMP_BUILD_TESTS OFF"
        "ASSIMP_BUILD_SAMPLES OFF"
        "ASSIMP_BUILD_DOCS OFF"
        "ASSIMP_BUILD_ASSIMP_VIEW OFF"

        "USE_STATIC_CRT ${RAWRBOX_BUILD_MSVC_MULTITHREADED_RUNTIME}"
        "ASSIMP_WARNINGS_AS_ERRORS OFF"

        "ASSIMP_INSTALL OFF"
        "ASSIMP_INSTALL_PDB OFF"

        "ASSIMP_BUILD_DRACO ${ASSIMP_BUILD_DRACO}"
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

if(assimp_ADDED)
    set_lib_runtime_mt(assimp)
endif()
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
set_property(TARGET ${output_target} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
target_link_libraries(${output_target}
    PUBLIC
        RAWRBOX.RENDER
        ${RAWRBOX_EXTRA_LIBS}

        assimp::assimp
)

set_lib_runtime_mt(${output_target})
# --------------

# TEST ----
# include(../cmake/catch2.cmake)
# --------------
