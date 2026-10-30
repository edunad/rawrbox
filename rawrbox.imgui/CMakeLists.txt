# Project setup
project("RAWRBOX.IMGUI" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - IMGUI lib" LANGUAGES CXX)
set(output_target RAWRBOX.IMGUI)

# LIBS ---
CPMAddPackage(
        NAME
        imgui
        GITHUB_REPOSITORY
        ocornut/imgui
        GIT_TAG
        v1.93.0-docking
        VERSION
        1.93.0
        DOWNLOAD_ONLY
        YES
)

if (imgui_ADDED)
    set(IMGUI_SRC
            ${imgui_SOURCE_DIR}/imgui.cpp
            ${imgui_SOURCE_DIR}/imgui_draw.cpp
            ${imgui_SOURCE_DIR}/imgui_tables.cpp
            ${imgui_SOURCE_DIR}/imgui_widgets.cpp
            ${imgui_SOURCE_DIR}/imgui_demo.cpp

            ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    )

    # Setup config file
    configure_file(./imconfig.h ${imgui_SOURCE_DIR}/imconfig.h)

    set(IMGUI_GLFW_BACKEND ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp)
    file(READ ${IMGUI_GLFW_BACKEND} IMGUI_GLFW_SOURCE)

    string(FIND "${IMGUI_GLFW_SOURCE}" "glfwGetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH)" IMGUI_GLFW_PATCHED)
    if (IMGUI_GLFW_PATCHED EQUAL -1)
        string(REPLACE
                "glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, window_no_input);"
                "if (glfwGetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH) != (window_no_input ? GLFW_TRUE : GLFW_FALSE)) glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, window_no_input);"
                IMGUI_GLFW_SOURCE "${IMGUI_GLFW_SOURCE}")
        file(WRITE ${IMGUI_GLFW_BACKEND} "${IMGUI_GLFW_SOURCE}")
    endif ()

    add_library(IMGUI STATIC ${IMGUI_SRC})
    target_include_directories(IMGUI
            PUBLIC
            $<BUILD_INTERFACE:${imgui_SOURCE_DIR}>
            $<BUILD_INTERFACE:${imgui_SOURCE_DIR}>/backends
    )

    target_link_libraries(IMGUI PUBLIC glfw)
    set_lib_runtime_mt(IMGUI)
endif ()

if (RAWRBOX_BUILD_RAWRBOX_IMGUI_IMPLOT)
    CPMAddPackage(
            NAME
            implot
            GITHUB_REPOSITORY
            epezent/implot
            GIT_TAG
            v1.1
            DOWNLOAD_ONLY
            YES
    )

    if (implot_ADDED)
        add_library(IMPLOT STATIC
                ${implot_SOURCE_DIR}/implot.cpp
                ${implot_SOURCE_DIR}/implot_items.cpp
                ${implot_SOURCE_DIR}/implot_demo.cpp
        )

        target_include_directories(IMPLOT PUBLIC $<BUILD_INTERFACE:${implot_SOURCE_DIR}>)
        target_link_libraries(IMPLOT PUBLIC IMGUI)
        set_lib_runtime_mt(IMPLOT)
    endif ()
endif ()

if (RAWRBOX_BUILD_RAWRBOX_IMGUI_RICHMD)
    add_library(imgui ALIAS IMGUI)

    CPMAddPackage(
            NAME
            imgui_rich_md
            GITHUB_REPOSITORY
            pthom/imgui_rich_md
            GIT_TAG
            v1.0.0
            OPTIONS
            "IMGUI_RICHMD_WITH_MERMAID ON"
            "IMGUI_RICHMD_EMBED_ASSETS ON"
            "IMGUI_RICHMD_BUILD_EXAMPLES OFF"
            "IMGUI_RICHMD_BUILD_TESTS OFF"
            "IMGUI_RICHMD_STB_IMAGE_IMPLEMENTATION OFF"
    )

    if (imgui_rich_md_ADDED)
        target_link_libraries(imgui_rich_md PRIVATE RAWRBOX.RENDER)
        set_lib_runtime_mt(imgui_rich_md)
    endif ()
endif ()
# ------------------
# ----

# LIB ----
file(GLOB_RECURSE RAWRBOX_IMGUI_IMPORTS "src/*.cpp" "include/*.hpp")
add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_IMGUI_IMPORTS})
target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_IMGUI)
target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})

target_link_libraries(${output_target} PUBLIC RAWRBOX.RENDER IMGUI)

if (RAWRBOX_BUILD_RAWRBOX_IMGUI_IMPLOT)
    target_compile_definitions(${output_target} PUBLIC RAWRBOX_IMGUI_IMPLOT)
    target_link_libraries(${output_target} PUBLIC IMPLOT)
endif ()

if (RAWRBOX_BUILD_RAWRBOX_IMGUI_RICHMD)
    target_compile_definitions(${output_target} PUBLIC RAWRBOX_IMGUI_RICHMD)
    target_link_libraries(${output_target} PUBLIC imgui_rich_md)
endif ()

set_lib_runtime_mt(${output_target})
# --------------

# TEST ----
#include(../cmake/catch2.cmake)
# --------------