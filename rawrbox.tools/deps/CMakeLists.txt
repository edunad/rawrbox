# Always build encoders optimized ---
if (MSVC)
    set(CMAKE_C_FLAGS_DEBUG "/Zi /O2 /Ob2")
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi /O2 /Ob2")
else ()
    set(CMAKE_C_FLAGS_DEBUG "-g -O2")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O2")
endif ()
# ----

# BASISU ---
CPMAddPackage(
        NAME
        basisu
        GITHUB_REPOSITORY
        BinomialLLC/basis_universal
        GIT_TAG
        v2_50
        DOWNLOAD_ONLY
        YES)

set(BASISU_EXAMPLES OFF)
add_subdirectory(${basisu_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/basisu EXCLUDE_FROM_ALL)

# Use the zstd from RAWRBOX.UTILS instead
get_target_property(BASISU_ENCODER_SOURCES basisu_encoder SOURCES)
list(FILTER BASISU_ENCODER_SOURCES EXCLUDE REGEX "zstd/zstd\\.c$")
set_property(TARGET basisu_encoder PROPERTY SOURCES ${BASISU_ENCODER_SOURCES})

target_include_directories(basisu_encoder PUBLIC ${basisu_SOURCE_DIR})
target_link_libraries(basisu_encoder PUBLIC libzstd_static)
set_lib_runtime_mt(basisu_encoder)
# ----

# GLTFPACK ---
CPMAddPackage("gh:zeux/meshoptimizer@1.3")

add_library(
        gltfpack_lib STATIC
        ${meshoptimizer_SOURCE_DIR}/gltf/animation.cpp
        ${meshoptimizer_SOURCE_DIR}/gltf/encodebasis.cpp
        ${meshoptimizer_SOURCE_DIR}/gltf/encodewebp.cpp
        ${meshoptimizer_SOURCE_DIR}/gltf/fileio.cpp
        ${meshoptimizer_SOURCE_DIR}/gltf/gltfpack.cpp
        ${meshoptimizer_SOURCE_DIR}/gltf/image.cpp
        ${meshoptimizer_SOURCE_DIR}/gltf/json.cpp
        ${meshoptimizer_SOURCE_DIR}/gltf/material.cpp
        ${meshoptimizer_SOURCE_DIR}/gltf/mesh.cpp
        ${meshoptimizer_SOURCE_DIR}/gltf/node.cpp
        ${meshoptimizer_SOURCE_DIR}/gltf/parseobj.cpp
        ${meshoptimizer_SOURCE_DIR}/gltf/parselib.cpp
        ${meshoptimizer_SOURCE_DIR}/gltf/parsegltf.cpp
        ${meshoptimizer_SOURCE_DIR}/gltf/stream.cpp
        ${meshoptimizer_SOURCE_DIR}/gltf/write.cpp)

set_source_files_properties(${meshoptimizer_SOURCE_DIR}/gltf/gltfpack.cpp PROPERTIES COMPILE_DEFINITIONS "main=gltfpack_main")
target_include_directories(gltfpack_lib PUBLIC ${meshoptimizer_SOURCE_DIR}/gltf)
target_link_libraries(gltfpack_lib PUBLIC meshoptimizer)

set_lib_runtime_mt(gltfpack_lib)
# ----

# LIBWEBP ---
CPMAddPackage(
        NAME
        libwebp
        GITHUB_REPOSITORY
        webmproject/libwebp
        VERSION
        1.6.0
        OPTIONS
        "BUILD_SHARED_LIBS OFF"
        "WEBP_BUILD_ANIM_UTILS OFF"
        "WEBP_BUILD_CWEBP OFF"
        "WEBP_BUILD_DWEBP OFF"
        "WEBP_BUILD_GIF2WEBP OFF"
        "WEBP_BUILD_IMG2WEBP OFF"
        "WEBP_BUILD_VWEBP OFF"
        "WEBP_BUILD_WEBPINFO OFF"
        "WEBP_BUILD_WEBPMUX OFF"
        "WEBP_BUILD_LIBWEBPMUX ON"
        "WEBP_USE_THREAD ON"
        "WEBP_NEAR_LOSSLESS ON"
        "WEBP_LINK_STATIC ON"
        "WEBP_BUILD_EXTRAS OFF")
# ----
