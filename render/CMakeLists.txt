project("RAWRBOX.RENDER" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - Render lib" LANGUAGES CXX)
set(output_target RAWRBOX.RENDER)
# --------------

# Packages ----
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

CPMAddPackage(
    NAME
        bgfx
    GITHUB_REPOSITORY
        edunad/bgfx.cmake
    GIT_TAG
        master
    OPTIONS
        "BGFX_BUILD_TOOLS ON"
        "BGFX_BUILD_TOOLS_SHADER ON"
        "BGFX_BUILD_TOOLS_GEOMETRY OFF"
        "BGFX_BUILD_TOOLS_BIN2C OFF"
        "BGFX_BUILD_TOOLS_TEXTURE OFF"
        "BGFX_BUILD_EXAMPLES OFF"
        "BGFX_INSTALL_EXAMPLES OFF"
        "BGFX_DEAR_IMGUI OFF"
        "BGFX_BUILD_TESTS OFF"
        "BGFX_OPENGLES_VERSION 43"
        "BGFX_OPENGL_VERSION 43"
        "BGFX_AMALGAMATED ON"
        "BX_AMALGAMATED ON")

if(bgfx_ADDED)
    message(STATUS "Including bgfx utils")
    include("../cmake/bgfx_utils.cmake")

    set_lib_runtime_mt(bimg)
    set_lib_runtime_mt(bx)
    set_lib_runtime_mt(bgfx)
endif()

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
# ----

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

# Compile shaders
if(WIN32)
    set(PROFILES 120 300_es spirv s_5_0)
    message(STATUS "Setting shader compilation to WIN")
else()
    set(PROFILES 120 300_es spirv)
    message(STATUS "Setting shader compilation to LINUX")
endif()

add_shaders_directory(./shaders RAWRBOX_SHADERS_TARGET_NAME)
message(STATUS "SHADERS: ${RAWRBOX_SHADERS_TARGET_NAME}")


# ------------------
# Copy custom shaders includes
if(RAWRBOX_COPY_SHADER_INCLUDES)
    file(COPY ${CMAKE_CURRENT_LIST_DIR}/shaders/include/ DESTINATION ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_SHADERS_FOLDER}/include/rawrbox/)
endif()
# ------------------

# ------------------

# Project setup
add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_RENDER_SOURCES})

copy_resources(TARGET ${output_target})
if(RAWRBOX_BUILD_RAWRBOX_SCRIPTING)
    copy_lua_libs(TARGET ${output_target})
endif()

target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_RENDER)
target_include_directories(${output_target} PUBLIC "include" ${libwebp_SOURCE_DIR}/src)
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(
    ${output_target}
    PUBLIC ${RAWRBOX_SHADERS_TARGET_NAME}
           RAWRBOX.MATH
           RAWRBOX.ENGINE
           ${RAWRBOX_EXTRA_LIBS}
           magic_enum::magic_enum
           utf8cpp
           bgfx
           bx
           lunasvg
           webpdemux
           glfw)

set_lib_runtime_mt(${output_target})

# TEST ----
include(../cmake/catch2.cmake)
# --------------
