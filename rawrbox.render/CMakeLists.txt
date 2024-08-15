project("RAWRBOX.RENDER" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - Render lib" LANGUAGES CXX)
set(output_target RAWRBOX.RENDER)
# --------------

# Packages ----
set(DXC_VERSION "v1.8.2405")
set(DXC_RELEASE_DATE "2024_05_24")

find_package(DXC REQUIRED)
# --------------

# Packages ----
CPMAddPackage(
    NAME
        glfw
    GITHUB_REPOSITORY
        glfw/glfw
    GIT_TAG
        3.4
    OPTIONS
        "GLFW_BUILD_DOCS OFF"
        "GLFW_BUILD_EXAMPLES OFF"
        "GLFW_BUILD_TESTS OFF"
        "GLFW_INSTALL OFF"
        "GLFW_USE_OSMESA OFF"
        "GLFW_VULKAN_STATIC OFF"
        "USE_MSVC_RUNTIME_LIBRARY_DLL OFF")

CPMAddPackage(
    NAME
        DiligentCore
    GITHUB_REPOSITORY
        DiligentGraphics/DiligentCore
    GIT_TAG
        master
    PATCH_COMMAND
        git restore Graphics/HLSL2GLSLConverterLib/src/HLSL2GLSLConverterImpl.cpp
    COMMAND
        git restore Graphics/GraphicsEngineVulkan/src/VulkanUtilities/VulkanInstance.cpp
    OPTIONS
        "DILIGENT_NO_RENDER_STATE_PACKAGER ON"
        "DILIGENT_NO_FORMAT_VALIDATION ON"
        "DILIGENT_NO_OPENGL ON"
        "DILIGENT_NO_VULKAN ${RAWRBOX_DISABLE_SUPPORT_VULKAN}"
        "DILIGENT_NO_DIRECT3D11 ON"
        "DILIGENT_NO_DIRECT3D12 ${RAWRBOX_DISABLE_SUPPORT_DX12}"
        "DILIGENT_BUILD_TESTS OFF"
)

if(DiligentCore_ADDED)
    set_lib_runtime_mt(SPIRV)
    set_lib_runtime_mt(SPIRV-Tools-static)
    set_lib_runtime_mt(SPIRV-Tools-opt)

    set_lib_runtime_mt(Diligent-BuildSettings)
    set_lib_runtime_mt(Diligent-PublicBuildSettings)
    set_lib_runtime_mt(Diligent-BasicPlatform)
    set_lib_runtime_mt(Diligent-GraphicsEngine)
    set_lib_runtime_mt(Diligent-Common)
    set_lib_runtime_mt(Diligent-ShaderTools)
    set_lib_runtime_mt(Diligent-GraphicsTools)
    set_lib_runtime_mt(Diligent-GraphicsEngineD3DBase)
    set_lib_runtime_mt(Diligent-GraphicsAccessories)
    set_lib_runtime_mt(Diligent-HLSL2GLSLConverterLib)
    set_lib_runtime_mt(Diligent-Primitives)

    set_lib_runtime_mt(Diligent-Archiver-static)
    set_lib_runtime_mt(Diligent-Archiver-shared)

    set_lib_runtime_mt(MachineIndependent)
    set_lib_runtime_mt(GenericCodeGen)
    set_lib_runtime_mt(spirv-cross-core)

    if(WIN32)
        set_lib_runtime_mt(Diligent-Win32Platform)
    else()
        set_lib_runtime_mt(Diligent-LinuxPlatform)

        set(DXC_SPIRV_PATH "" CACHE STRING "" FORCE)
        set(DILIGENT_DXCOMPILER_FOR_SPIRV_PATH "" CACHE STRING "" FORCE)
    endif()
endif()

CPMAddPackage("gh:nemtrif/utfcpp@4.0.5")
if(utfcpp_ADDED)
    set_lib_runtime_mt(utf8cpp)
endif()

CPMAddPackage("gh:sammycage/lunasvg@2.4.0")
if(lunasvg_ADDED)
    set_lib_runtime_mt(lunasvg)
endif()

CPMAddPackage(
    NAME
        libwebp
    GITHUB_REPOSITORY
        webmproject/libwebp
    VERSION
        1.4.0
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
        "WEBP_BUILD_EXTRAS OFF")

if(libwebp_ADDED)
    set_lib_runtime_mt(webp)
    set_lib_runtime_mt(webpdemux)
    set_lib_runtime_mt(libwebpmux)
    #set_lib_runtime_mt(sharpyuv)
    #set_lib_runtime_mt(xxhash)
endif()
# ------------


# Grab source files
file(GLOB_RECURSE RAWRBOX_RENDER_SOURCES "src/*.cpp" "src/*.c" "include/*.h" "include/*.hpp")

if(RAWRBOX_BUILD_RAWRBOX_RESOURCES)
    list(APPEND RAWRBOX_EXTRA_LIBS RAWRBOX.RESOURCES)
else()
    remove_folder("${RAWRBOX_RENDER_SOURCES}" "/resources/")
    set(RAWRBOX_RENDER_SOURCES "${CLEAN_SOURCES}")
endif()

if(RAWRBOX_BUILD_RAWRBOX_SCRIPTING)
    list(APPEND RAWRBOX_EXTRA_LIBS RAWRBOX.SCRIPTING)
    message(STATUS "Enabling RAWRBOX.RENDER scripting plugin")
else()
    remove_folder("${RAWRBOX_RENDER_SOURCES}" "/scripting/")
    set(RAWRBOX_RENDER_SOURCES "${CLEAN_SOURCES}")
endif()
# -----------------


# Project setup
add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_RENDER_SOURCES})

copy_resources(TARGET ${output_target})
if(RAWRBOX_BUILD_RAWRBOX_SCRIPTING)
    copy_lua_libs(TARGET ${output_target})
endif()

target_compile_options(${output_target} PRIVATE -DENGINE_DLL)
target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_RENDER)
target_include_directories(${output_target} PUBLIC "include" "${libwebp_SOURCE_DIR}/src")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
set_property(TARGET ${output_target} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
target_link_libraries(
    ${output_target}
    PUBLIC
            # Diligent -------------------
            Diligent-Common
            Diligent-GraphicsAccessories
            Diligent-GraphicsTools
            Diligent-TargetPlatform
            Diligent-PublicBuildSettings
            # ----------------------------

            # RAWRBOX --------------------
            RAWRBOX.MATH
            RAWRBOX.ENGINE
            ${RAWRBOX_EXTRA_LIBS}
            # ----------------------------

            utf8cpp
            lunasvg
            webpdemux
            glfw
)

set_lib_runtime_mt(${output_target})

if(WIN32)
    set(DILIGENT_LINK_MODE shared)
    message(STATUS "Setting deligent mode to: shared")

    if(D3D12_SUPPORTED)
        message(STATUS "Enabling DX12 support")
        target_link_libraries(${output_target} PRIVATE Diligent-GraphicsEngineD3D12-${DILIGENT_LINK_MODE})
        target_compile_definitions(${output_target} PUBLIC RAWRBOX_SUPPORT_DX12)

        set_lib_runtime_mt(Diligent-GraphicsEngineD3D12-shared)
        set_lib_runtime_mt(Diligent-GraphicsEngineD3D12-static)
    endif()

    if(D3D11_SUPPORTED)
        message(STATUS "Enabling DX11 support")
        target_link_libraries(${output_target} PRIVATE Diligent-GraphicsEngineD3D11-${DILIGENT_LINK_MODE})
        target_compile_definitions(${output_target} PUBLIC RAWRBOX_SUPPORT_DX11)

        set_lib_runtime_mt(Diligent-GraphicsEngineD3D11-shared)
        set_lib_runtime_mt(Diligent-GraphicsEngineD3D11-static)
    endif()

else()
    set(DILIGENT_LINK_MODE static)
    message(STATUS "Setting deligent mode to: static")
endif()

if(VULKAN_SUPPORTED)
    message(STATUS "Enabling VULKAN support")

    target_link_libraries(${output_target} PRIVATE Diligent-GraphicsEngineVk-${DILIGENT_LINK_MODE})
    target_compile_definitions(${output_target} PUBLIC RAWRBOX_SUPPORT_VULKAN)

    set_lib_runtime_mt(Diligent-GraphicsEngineVk-shared)
    set_lib_runtime_mt(Diligent-GraphicsEngineVk-static)
endif()

if(GL_SUPPORTED)
    message(STATUS "Enabling OPENGL support")

    target_link_libraries(${output_target} PRIVATE Diligent-GraphicsEngineOpenGL-${DILIGENT_LINK_MODE})
    target_compile_definitions(${output_target} PUBLIC RAWRBOX_SUPPORT_OPENGL)

    set_lib_runtime_mt(Diligent-GraphicsEngineOpenGL-shared)
    set_lib_runtime_mt(Diligent-GraphicsEngineOpenGL-static)
endif()

# COPY DXC ---------------------------------
add_custom_target(
    copy_dxc ALL COMMAND ${CMAKE_COMMAND} -E copy ${DXC_BINARY} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "Copying dxcompiler lib into binary directory")

add_custom_target(
    copy_dxil ALL COMMAND ${CMAKE_COMMAND} -E copy ${DXC_BINARY_IL}
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "Copying dxil lib into binary directory")

add_dependencies(${output_target} copy_dxc copy_dxil)
# ---------------------------------

# TEST ----
#include(../cmake/catch2.cmake)
# --------------
