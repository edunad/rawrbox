# CPM Package Lock
# This file should be committed to version control

# fmt
CPMDeclarePackage(fmt
  GIT_TAG 10.2.1
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
# glaze
CPMDeclarePackage(glaze
  VERSION 2.1.7
  GITHUB_REPOSITORY stephenberry/glaze
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
# zlib
CPMDeclarePackage(zlib
  NAME zlib
  GIT_TAG 2.1.6
  GITHUB_REPOSITORY zlib-ng/zlib-ng
  OPTIONS
    "ZLIB_COMPAT ON"
    "ZLIB_BUILD_EXAMPLES OFF"
    "ZLIB_ENABLE_TESTS OFF"
    "POSITION_INDEPENDENT_CODE ON"
    "WITH_GTEST OFF"
  USE_CACHE OFF
)
# Catch2
CPMDeclarePackage(Catch2
  NAME Catch2
  VERSION 3.5.2
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
  VERSION 4.0.1
  GITHUB_REPOSITORY bshoshany/thread-pool
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
# cpptrace
CPMDeclarePackage(cpptrace
  VERSION 0.4.0
  GITHUB_REPOSITORY jeremy-rifkin/cpptrace
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
# LuaBridge3 (unversioned)
# CPMDeclarePackage(LuaBridge3
#  NAME LuaBridge3
#  GIT_TAG master
#  GITHUB_REPOSITORY kunitoki/LuaBridge3
#  OPTIONS
#    "LUABRIDGE_TESTING OFF"
#    "LUABRIDGE_COVERAGE OFF"
#)
# luau
CPMDeclarePackage(luau
  NAME luau
  GIT_TAG 0.613
  GITHUB_REPOSITORY luau-lang/luau
  OPTIONS
    "LUAU_BUILD_CLI OFF"
    "LUAU_BUILD_TESTS OFF"
    "LUAU_BUILD_BENCHMARKS OFF"
    "LUAU_BUILD_WEB OFF"
)
# libcpr
CPMDeclarePackage(libcpr
  NAME libcpr
  GIT_TAG 1.10.5
  GITHUB_REPOSITORY libcpr/cpr
  OPTIONS
    "BUILD_SHARED_LIBS OFF"
    "CPR_ENABLE_CURL_HTTP_ONLY ON"
    "CPR_ENABLE_SSL ON"
    "CURL_ZLIB ON"
  PATCH_COMMAND git apply --reject --whitespace=fix D:/rawrbox/rawrbox.network/patch/libcpr.patch
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
#  OPTIONS
#    "DILIGENT_NO_OPENGL ON"
#    "DILIGENT_NO_VULKAN OFF"
#    "DILIGENT_NO_DIRECT3D11 ON"
#    "DILIGENT_NO_DIRECT3D12 OFF"
#    "DILIGENT_BUILD_TESTS OFF"
#  PATCH_COMMAND git restore Graphics/HLSL2GLSLConverterLib/src/HLSL2GLSLConverterImpl.cpp COMMAND git restore Graphics/GraphicsEngineVulkan/src/VulkanUtilities/VulkanInstance.cpp
#)
# utfcpp
CPMDeclarePackage(utfcpp
  VERSION 4.0.5
  GITHUB_REPOSITORY nemtrif/utfcpp
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
# lunasvg
CPMDeclarePackage(lunasvg
  VERSION 2.3.9
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
# libwebm (unversioned)
# CPMDeclarePackage(libwebm
#  NAME libwebm
#  GIT_TAG libwebm-1.0.0.31
#  GITHUB_REPOSITORY webmproject/libwebm
#  OPTIONS
#    "BUILD_SHARED_LIBS OFF"
#    "ENABLE_WEBMTS OFF"
#    "ENABLE_WEBMINFO OFF"
#    "ENABLE_WEBM_PARSER OFF"
#    "ENABLE_TESTS OFF"
#    "ENABLE_SAMPLE_PROGRAMS OFF"
#)
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
#    "ASSIMP_BUILD_ASSIMP_VIEW OFF"
#    "USE_STATIC_CRT ON"
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
# Jolt
CPMDeclarePackage(Jolt
  NAME Jolt
  VERSION 4.0.2
  GITHUB_REPOSITORY jrouwe/JoltPhysics
  SOURCE_SUBDIR Build
  OPTIONS
    "COMPILE_AS_SHARED_LIBRARY OFF"
    "INTERPROCEDURAL_OPTIMIZATION ON"
    "USE_STATIC_MSVC_RUNTIME_LIBRARY ON"
    "ENABLE_ALL_WARNINGS OFF"
)
# muli (unversioned)
# CPMDeclarePackage(muli
#  NAME muli
#  GIT_TAG master
#  GITHUB_REPOSITORY Sopiro/Muli
#  OPTIONS
#    "MULI_BUILD_DEMO OFF"
#)
