project("RAWRBOX.RENDER" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - Render lib" LANGUAGES CXX)
set(output_target RAWRBOX.RENDER)
# --------------

# Packages ----
add_subdirectory(DiligentCore)

CPMAddPackage(
    NAME
        glfw
    GITHUB_REPOSITORY
        glfw/glfw
    GIT_TAG
        3.3.8
    OPTIONS
        "GLFW_BUILD_DOCS OFF"
        "GLFW_BUILD_EXAMPLES OFF"
        "GLFW_BUILD_TESTS OFF"
        "GLFW_INSTALL OFF"
        "GLFW_USE_OSMESA OFF"
        "GLFW_VULKAN_STATIC OFF"
        "USE_MSVC_RUNTIME_LIBRARY_DLL OFF")


CPMAddPackage("gh:nemtrif/utfcpp@3.2.4")
if(utfcpp_ADDED)
    set_lib_runtime_mt(utf8cpp)
endif()

CPMAddPackage("gh:sammycage/lunasvg@2.3.8")
if(lunasvg_ADDED)
    set_lib_runtime_mt(lunasvg)
endif()

CPMAddPackage(
    NAME
        libwebp
    GITHUB_REPOSITORY
        webmproject/libwebp
    VERSION
        1.3.2
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
        "WEBP_BUILD_EXTRAS OFF")

if(libwebp_ADDED)
    set_lib_runtime_mt(webp)
    set_lib_runtime_mt(webpdemux)
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
    if(NOT RAWRBOX_BUILD_RAWRBOX_RESOURCES) # RAWRBOX.RESOURCES already includes scripting if RAWRBOX_BUILD_RAWRBOX_SCRIPTING is enabled
        list(APPEND RAWRBOX_EXTRA_LIBS RAWRBOX.SCRIPTING)
    endif()
else()
    remove_folder("${RAWRBOX_RENDER_SOURCES}" "/scripting/")
    set(RAWRBOX_RENDER_SOURCES "${CLEAN_SOURCES}")
endif()
# -----------------

## RESOURCES -----
file(GLOB_RECURSE RAWRBOX_RENDER_SHADERS "content/*.psh" "content/*.vsh")
# ----------------

# Project setup
add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_RENDER_SOURCES})

#copy_resources(TARGET ${output_target})
#if(RAWRBOX_BUILD_RAWRBOX_SCRIPTING)
#    copy_lua_libs(TARGET ${output_target})
#endif()

target_compile_options(${output_target} PRIVATE -DUNICODE -DENGINE_DLL)
target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_RENDER)
target_include_directories(${output_target} PUBLIC "include" "${libwebp_SOURCE_DIR}/src" "./DiligentCore")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(
    ${output_target}
    PUBLIC ${RAWRBOX_RENDER_SHADERS}
           Diligent-GraphicsTools
           #Diligent-ShaderTools
           RAWRBOX.MATH
           RAWRBOX.ENGINE
           ${RAWRBOX_EXTRA_LIBS}
           magic_enum::magic_enum
           utf8cpp
           lunasvg
           webpdemux
           glfw)

#set_lib_runtime_mt(${output_target})

if(WIN32)
    set(DILIGENT_LINK_MODE shared)
    message(STATUS "Setting deligent mode to: shared")

    if(D3D11_SUPPORTED)
        target_link_libraries(${output_target} PRIVATE Diligent-GraphicsEngineD3D11-${DILIGENT_LINK_MODE})
    endif()

    if(D3D12_SUPPORTED)
        target_link_libraries(${output_target} PRIVATE Diligent-GraphicsEngineD3D12-${DILIGENT_LINK_MODE})
    endif()
else()
    set(DILIGENT_LINK_MODE static)
    message(STATUS "Setting deligent mode to: static")
endif()

if(GL_SUPPORTED)
    target_link_libraries(${output_target} PRIVATE Diligent-GraphicsEngineOpenGL-${DILIGENT_LINK_MODE})
endif()

if(VULKAN_SUPPORTED)
    target_link_libraries(${output_target} PRIVATE Diligent-GraphicsEngineVk-${DILIGENT_LINK_MODE})
endif()

if(WIN32)
    copy_required_dlls(${output_target})
endif()

# TEST ----
#include(../cmake/catch2.cmake)
# --------------
