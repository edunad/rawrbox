# CPM Package Lock
# This file should be committed to version control

# fmt
CPMDeclarePackage(fmt
  GIT_TAG 10.1.1
  GITHUB_REPOSITORY fmtlib/fmt
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
# magic_enum
CPMDeclarePackage(magic_enum
  VERSION 0.9.5
  GITHUB_REPOSITORY Neargye/magic_enum
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
# nlohmann_json
CPMDeclarePackage(nlohmann_json
  NAME nlohmann_json
  VERSION 3.11.3
  URL
    "https://github.com/nlohmann/json/releases/download/v3.11.3/include.zip"
    "URL_HASH"
    "SHA256=a22461d13119ac5c78f205d3df1db13403e58ce1bb1794edc9313677313f4a9d"
)
# ZLIB
CPMDeclarePackage(ZLIB
  NAME ZLIB
  GIT_TAG v1.3
  GITHUB_REPOSITORY madler/zlib
  OPTIONS
    "ASM686 FALSE"
    "AMD64 FALSE"
    "POSITION_INDEPENDENT_CODE TRUE"
)
# Catch2
CPMDeclarePackage(Catch2
  NAME Catch2
  VERSION 3.5.0
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
# cpptrace
CPMDeclarePackage(cpptrace
  VERSION 0.3.1
  GITHUB_REPOSITORY jeremy-rifkin/cpptrace
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
    "CURL_ZLIB ON"
)
# glfw
CPMDeclarePackage(glfw
  NAME glfw
  GIT_TAG 3.3.9
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
# DiligentCore (unversioned)
# CPMDeclarePackage(DiligentCore
#  NAME DiligentCore
#  GIT_TAG master
#  GITHUB_REPOSITORY DiligentGraphics/DiligentCore
#  PATCH_COMMAND git restore Graphics/HLSL2GLSLConverterLib/src/HLSL2GLSLConverterImpl.cpp COMMAND git restore Graphics/GraphicsEngineVulkan/src/VulkanUtilities/VulkanInstance.cpp
#)
# utfcpp
CPMDeclarePackage(utfcpp
  VERSION 4.0.4
  GITHUB_REPOSITORY nemtrif/utfcpp
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
# lunasvg
CPMDeclarePackage(lunasvg
  VERSION 2.3.8
  GITHUB_REPOSITORY sammycage/lunasvg
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
# libwebp
CPMDeclarePackage(libwebp
  NAME libwebp
  VERSION 1.3.2
  GITHUB_REPOSITORY webmproject/libwebp
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
    "WEBP_BUILD_EXTRAS OFF"
)
# assimp (unversioned)
# CPMDeclarePackage(assimp
#  NAME assimp
#  GIT_TAG master
#  GITHUB_REPOSITORY edunad/assimp
#  OPTIONS
#    "BUILD_SHARED_LIBS OFF"
#    "ASSIMP_NO_EXPORT ON"
#    "ASSIMP_BUILD_ZLIB OFF"
#    "ASSIMP_BUILD_ASSIMP_TOOLS OFF"
#    "ASSIMP_BUILD_TESTS OFF"
#    "ASSIMP_BUILD_SAMPLES OFF"
#    "ASSIMP_BUILD_DOCS OFF"
#    "USE_STATIC_CRT OFF"
#    "ASSIMP_BUILD_ASSIMP_VIEW OFF"
#    "ASSIMP_WARNINGS_AS_ERRORS OFF"
#    "ASSIMP_INSTALL OFF"
#    "ASSIMP_INSTALL_PDB OFF"
#    "ASSIMP_BUILD_DRACO ON"
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
# clip
CPMDeclarePackage(clip
  NAME clip
  VERSION 1.7
  GITHUB_REPOSITORY dacap/clip
  OPTIONS
    "CLIP_EXAMPLES OFF"
    "CLIP_TESTS OFF"
)
