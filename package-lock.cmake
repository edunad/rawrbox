# CPM Package Lock
# This file should be committed to version control

# fmt
CPMDeclarePackage(fmt
  GIT_TAG 10.0.0
  GITHUB_REPOSITORY fmtlib/fmt
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
# magic_enum
CPMDeclarePackage(magic_enum
  VERSION 0.9.0
  GITHUB_REPOSITORY Neargye/magic_enum
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
# nlohmann_json
CPMDeclarePackage(nlohmann_json
  NAME nlohmann_json
  VERSION 3.11.2
  URL https://github.com/nlohmann/json/releases/download/v3.11.2/include.zip URL_HASH SHA256=e5c7a9f49a16814be27e4ed0ee900ecd0092bfb7dbfca65b5a421b774dccaaed
)
# zlib
CPMDeclarePackage(zlib
  NAME zlib
  GIT_TAG v1.2.13
  GITHUB_REPOSITORY madler/zlib
  OPTIONS
    "ASM686 FALSE"
    "AMD64 FALSE"
    "POSITION_INDEPENDENT_CODE TRUE"
)
# Catch2
CPMDeclarePackage(Catch2
  NAME Catch2
  VERSION 3.3.2
  GITHUB_REPOSITORY catchorg/Catch2
  OPTIONS
    "CATCH_INSTALL_DOCS OFF"
    "CATCH_INSTALL_EXTRAS ON"
)
# qhull
CPMDeclarePackage(qhull
  NAME qhull
  GIT_TAG 2020.2
  DOWNLOAD_ONLY ON
  GITHUB_REPOSITORY qhull/qhull
)
# thread-pool
CPMDeclarePackage(thread-pool
  VERSION 3.5.0
  GITHUB_REPOSITORY bshoshany/thread-pool
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
# libcpr
CPMDeclarePackage(libcpr
  NAME libcpr
  GIT_TAG 1.10.4
  GITHUB_REPOSITORY libcpr/cpr
  OPTIONS
    "BUILD_SHARED_LIBS OFF"
    "CPR_ENABLE_CURL_HTTP_ONLY ON"
    "CPR_ENABLE_SSL ON"
)
# glfw
CPMDeclarePackage(glfw
  NAME glfw
  GIT_TAG 3.3.8
  GITHUB_REPOSITORY glfw/glfw
  OPTIONS
    "GLFW_BUILD_DOCS OFF"
    "GLFW_BUILD_EXAMPLES OFF"
    "GLFW_BUILD_TESTS OFF"
    "GLFW_INSTALL OFF"
    "GLFW_USE_OSMESA OFF"
    "GLFW_VULKAN_STATIC OFF"
    "USE_MSVC_RUNTIME_LIBRARY_DLL OFF"
)
# bgfx (unversioned)
# CPMDeclarePackage(bgfx
#  NAME bgfx
#  GIT_TAG master
#  GITHUB_REPOSITORY edunad/bgfx.cmake
#  OPTIONS
#    "BGFX_BUILD_TOOLS ON"
#    "BGFX_BUILD_TOOLS_SHADER ON"
#    "BGFX_BUILD_TOOLS_GEOMETRY OFF"
#    "BGFX_BUILD_TOOLS_BIN2C OFF"
#    "BGFX_BUILD_TOOLS_TEXTURE OFF"
#    "BGFX_BUILD_EXAMPLES OFF"
#    "BGFX_INSTALL_EXAMPLES OFF"
#    "BGFX_DEAR_IMGUI OFF"
#    "BGFX_BUILD_TESTS OFF"
#    "BGFX_OPENGLES_VERSION 43"
#    "BGFX_OPENGL_VERSION 43"
#    "BGFX_AMALGAMATED ON"
#    "BX_AMALGAMATED ON"
#)
# utfcpp
CPMDeclarePackage(utfcpp
  VERSION 3.2.3
  GITHUB_REPOSITORY nemtrif/utfcpp
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
# assimp (unversioned)
# CPMDeclarePackage(assimp
#  NAME assimp
#  GIT_TAG kimkulling/prepare_version_5.2.6_rc1
#  GITHUB_REPOSITORY assimp/assimp
#  OPTIONS
#    "BUILD_SHARED_LIBS OFF"
#    "ASSIMP_NO_EXPORT ON"
#    "ASSIMP_BUILD_ZLIB OFF"
#    "ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF"
#    "ASSIMP_BUILD_ASSIMP_TOOLS OFF"
#    "ASSIMP_BUILD_TESTS OFF"
#    "ASSIMP_BUILD_SAMPLES OFF"
#    "ASSIMP_BUILD_DOCS OFF"
#    "USE_STATIC_CRT ON"
#    "ASSIMP_BUILD_ASSIMP_VIEW OFF"
#    "ASSIMP_WARNINGS_AS_ERRORS OFF"
#    "ASSIMP_INSTALL OFF"
#    "ASSIMP_INSTALL_PDB OFF"
#    "ASSIMP_BUILD_DRACO OFF"
#    "ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF"
#    "ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF"
#    "ASSIMP_BUILD_OBJ_IMPORTER ON"
#    "ASSIMP_BUILD_FBX_IMPORTER ON"
#    "ASSIMP_BUILD_DAE_IMPORTER ON"
#    "ASSIMP_BUILD_STL_IMPORTER ON"
#    "ASSIMP_BUILD_GLTF_IMPORTER ON"
#    "ASSIMP_BUILD_3DS_IMPORTER ON"
#    "ASSIMP_BUILD_MD5_IMPORTER ON"
#)
# libwebm (unversioned)
# CPMDeclarePackage(libwebm
#  NAME libwebm
#  GIT_TAG libwebm-1.0.0.30
#  GITHUB_REPOSITORY webmproject/libwebm
#  OPTIONS
#    "BUILD_SHARED_LIBS OFF"
#    "ENABLE_WEBMTS OFF"
#    "ENABLE_WEBMINFO OFF"
#    "ENABLE_WEBM_PARSER OFF"
#    "ENABLE_TESTS OFF"
#    "ENABLE_SAMPLE_PROGRAMS OFF"
#)
# clip
CPMDeclarePackage(clip
  NAME clip
  VERSION 1.5
  GITHUB_REPOSITORY dacap/clip
  OPTIONS
    "CLIP_EXAMPLES OFF"
    "CLIP_TESTS OFF"
)
# Jolt
CPMDeclarePackage(Jolt
  NAME Jolt
  VERSION 3.0.1
  GITHUB_REPOSITORY jrouwe/JoltPhysics
  OPTIONS
    "COMPILE_AS_SHARED_LIBRARY OFF"
    "INTERPROCEDURAL_OPTIMIZATION ON"
    "USE_STATIC_MSVC_RUNTIME_LIBRARY ON"
    "ENABLE_ALL_WARNINGS OFF"
    "SOURCE_SUBDIR"
    "Build"
)
# muli (unversioned)
# CPMDeclarePackage(muli
#  NAME muli
#  GIT_TAG master
#  GITHUB_REPOSITORY Sopiro/Muli
#  OPTIONS
#    "MULI_BUILD_DEMO OFF"
#)