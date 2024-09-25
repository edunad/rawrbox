# CPM Package Lock
# This file should be committed to version control

# fmt
CPMDeclarePackage(fmt
  GIT_TAG 11.0.2
  GITHUB_REPOSITORY fmtlib/fmt
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
# magic_enum
CPMDeclarePackage(magic_enum
  VERSION 0.9.6
  GITHUB_REPOSITORY Neargye/magic_enum
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
# glaze
CPMDeclarePackage(glaze
  VERSION 3.5.0
  GITHUB_REPOSITORY stephenberry/glaze
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
# Catch2
CPMDeclarePackage(Catch2
  NAME Catch2
  VERSION 3.7.1
  GITHUB_REPOSITORY catchorg/Catch2
  OPTIONS
    "CATCH_INSTALL_DOCS OFF"
    "CATCH_CONFIG_FAST_COMPILE ON"
    "CATCH_INSTALL_EXTRAS ON"
)
# thread-pool
CPMDeclarePackage(thread-pool
  VERSION 4.1.0
  GITHUB_REPOSITORY bshoshany/thread-pool
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
# meshoptimizer
CPMDeclarePackage(meshoptimizer
  VERSION 0.21
  GITHUB_REPOSITORY zeux/meshoptimizer
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
# glfw
CPMDeclarePackage(glfw
  NAME glfw
  GIT_TAG 3.4
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
#    "DILIGENT_NO_RENDER_STATE_PACKAGER ON"
#    "DILIGENT_NO_FORMAT_VALIDATION ON"
#    "DILIGENT_NO_OPENGL ON"
#    "DILIGENT_NO_VULKAN OFF"
#    "DILIGENT_NO_DIRECT3D11 ON"
#    "DILIGENT_NO_DIRECT3D12 OFF"
#    "DILIGENT_BUILD_TESTS OFF"
#  PATCH_COMMAND git restore Graphics/HLSL2GLSLConverterLib/src/HLSL2GLSLConverterImpl.cpp COMMAND git restore Graphics/GraphicsEngineVulkan/src/VulkanUtilities/VulkanInstance.cpp
#)
# ozz
CPMDeclarePackage(ozz
  NAME ozz
  GIT_TAG 0.15.0
  GITHUB_REPOSITORY guillaumeblanc/ozz-animation
  OPTIONS
    "ozz_build_tools OFF"
    "ozz_build_fbx OFF"
    "ozz_build_gltf OFF"
    "ozz_build_samples OFF"
    "ozz_build_howtos OFF"
    "ozz_build_tests OFF"
    "ozz_build_msvc_rt_dll ON"
  PATCH_COMMAND git apply --reject --whitespace=fix D:/rawrbox/rawrbox.render/patch/ozz.patch
)
# utfcpp
CPMDeclarePackage(utfcpp
  VERSION 4.0.5
  GITHUB_REPOSITORY nemtrif/utfcpp
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
# lunasvg
CPMDeclarePackage(lunasvg
  VERSION 2.4.1
  GITHUB_REPOSITORY sammycage/lunasvg
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
# libwebp
CPMDeclarePackage(libwebp
  NAME libwebp
  VERSION 1.4.0
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
    "WEBP_BUILD_LIBWEBPMUX ON"
    "WEBP_BUILD_LIBWEBPMUX ON"
    "WEBP_USE_THREAD ON"
    "WEBP_NEAR_LOSSLESS ON"
    "WEBP_LINK_STATIC ON"
    "WEBP_BUILD_EXTRAS OFF"
)
# fastgltf (unversioned)
# CPMDeclarePackage(fastgltf
#  NAME fastgltf
#  GIT_TAG main
#  GITHUB_REPOSITORY spnda/fastgltf
#  OPTIONS
#    "FASTGLTF_COMPILE_AS_CPP20 ON"
#)
