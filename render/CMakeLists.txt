project("RAWRBOX.RENDER" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - Render lib" LANGUAGES CXX)
set(output_target RAWRBOX.RENDER)

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

CPMAddPackage("gh:nemtrif/utfcpp@3.2.3")
# ----

if(RAWRBOX_ENABLE_WEBM_SUPPORT)
    message(STATUS "Enabled WEBM & VPX codec")

    CPMAddPackage(
        NAME
        libwebm
        GIT_TAG
        libwebm-1.0.0.30
        GITHUB_REPOSITORY
        webmproject/libwebm
        OPTIONS
        "BUILD_SHARED_LIBS OFF"
        "ENABLE_WEBMTS OFF"
        "ENABLE_WEBMINFO OFF"
        "ENABLE_WEBM_PARSER OFF"
        "ENABLE_TESTS OFF"
        "ENABLE_SAMPLE_PROGRAMS OFF")

    if(libwebm_ADDED)
        target_include_directories(webm PUBLIC ${libwebm_SOURCE_DIR})
        set_lib_runtime_mt(webm)
        set_lib_runtime_mt(mkvparser)

        list(APPEND RAWRBOX_EXTRA_LIBS webm)
    endif()

    # Add codec
    set(VPX_VERSION v1.13.0)
    find_package(VPX REQUIRED)

    if(WIN32)
        add_custom_target(
            copy_dll_vpx ALL COMMAND ${CMAKE_COMMAND} -E copy ${VPX_BINARY}
                                     ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}
            COMMENT "Copying VPX DLL into binary directory")

        list(APPEND RAWRBOX_EXTRA_DEPS copy_dll_vpx)
    endif()

    list(APPEND RAWRBOX_EXTRA_LIBS VPX::VPX)
    # --------------

endif()
# ----

if(RAWRBOX_BUILD_RAWRBOX_RESOURCES)
    list(APPEND RAWRBOX_EXTRA_LIBS RAWRBOX.RESOURCES)
endif()

# --------------

# Grab source files
file(GLOB_RECURSE RAWRBOX_RENDER_SOURCES "src/*.cpp" "src/*.c" "include/*.h" "include/*.hpp")

if(NOT RAWRBOX_BUILD_RAWRBOX_RESOURCES)
    foreach(TMP_PATH ${RAWRBOX_RENDER_SOURCES})
        string(FIND ${TMP_PATH} "/resources/" EXCLUDE_DIR_FOUND)
        if(NOT ${EXCLUDE_DIR_FOUND} EQUAL -1)
            list(REMOVE_ITEM RAWRBOX_RENDER_SOURCES ${TMP_PATH})
        endif()
    endforeach(TMP_PATH)
endif()

if(NOT RAWRBOX_ENABLE_WEBM_SUPPORT)
    foreach(TMP_PATH ${RAWRBOX_RENDER_SOURCES})
        string(FIND ${TMP_PATH} "/webm/" EXCLUDE_DIR_FOUND)
        if(NOT ${EXCLUDE_DIR_FOUND} EQUAL -1)
            list(REMOVE_ITEM RAWRBOX_RENDER_SOURCES ${TMP_PATH})
        endif()
    endforeach(TMP_PATH)
endif()

# -----------------

# Compile shaders
if(LINUX)
    set(PROFILES 120 300_es spirv)
else()
    set(PROFILES 120 300_es spirv s_5_0)
endif()

add_shaders_directory(./shaders RAWRBOX_SHADERS_TARGET_NAME)
message(STATUS "SHADERS: ${RAWRBOX_SHADERS_TARGET_NAME}")
# ------------------

# Copy content - RENDER
add_custom_target(
    copy_resources_render ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/${RAWRBOX_CONTENT_FOLDER}
                                      ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/${RAWRBOX_CONTENT_FOLDER}
    COMMENT "Copying RENDERER resources into binary directory")
# ------------------

# Project setup
add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_RENDER_SOURCES})
add_dependencies(${output_target} copy_resources_render ${RAWRBOX_EXTRA_DEPS})
target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_RENDER)
target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(
    ${output_target}
    PUBLIC ${RAWRBOX_SHADERS_TARGET_NAME}
           RAWRBOX.MATH
           RAWRBOX.ENGINE
           ${RAWRBOX_EXTRA_LIBS}
           fmt::fmt
           magic_enum::magic_enum
           utf8cpp
           bgfx
           bx
           glfw)

set_lib_runtime_mt(${output_target})

# TEST ----
include(../cmake/catch2.cmake)
# --------------
