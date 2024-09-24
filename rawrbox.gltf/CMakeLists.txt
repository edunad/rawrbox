# Project setup
project("RAWRBOX.GLTF" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - GLTF lib" LANGUAGES CXX)
set(output_target RAWRBOX.GLTF)

# LIBS ---
CPMAddPackage(
    NAME
        fastgltf
    GIT_TAG
        #v0.8.1
        main
    GITHUB_REPOSITORY
        spnda/fastgltf
    OPTIONS
        "FASTGLTF_COMPILE_AS_CPP20 ON"
    )

if(fastgltf_ADDED)
    set_lib_runtime_mt(fastgltf)
endif()

CPMAddPackage("gh:zeux/meshoptimizer@0.21")
# ----

# Grab source files
file(GLOB_RECURSE RAWRBOX_GLTF_IMPORTS "src/*.cpp" "include/*.hpp")

if(RAWRBOX_BUILD_RAWRBOX_RESOURCES)
    set(RAWRBOX_EXTRA_LIBS RAWRBOX.RESOURCES)
else()
    remove_folder("${RAWRBOX_GLTF_IMPORTS}" "/resources/")
    set(RAWRBOX_GLTF_IMPORTS "${CLEAN_SOURCES}")
endif()
# ----

add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_GLTF_IMPORTS})
target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_GLTF)
target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})

target_link_libraries(${output_target}
    PUBLIC
        RAWRBOX.RENDER
        ${RAWRBOX_EXTRA_LIBS}

        fastgltf
        meshoptimizer
)

set_lib_runtime_mt(${output_target})
# --------------

# TEST ----
# include(../cmake/catch2.cmake)
# --------------
