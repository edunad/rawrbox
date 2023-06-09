# Project setup
project(
    "RAWRBOX.UI"
    VERSION ${RAWRBOX_VERSION}
    DESCRIPTION "RawrBox - UI lib"
    LANGUAGES CXX)
set(output_target RAWRBOX.UI)

# Copy content - UI
add_custom_target(
    copy_resources_ui ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/${RAWRBOX_CONTENT_FOLDER}
            ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/${RAWRBOX_CONTENT_FOLDER}
    COMMENT "Copying UI resources into binary directory")
# ------------------

# LIBS ---
CPMAddPackage("gh:dacap/clip@1.5")
if(clip_ADDED)
    target_include_directories(clip PUBLIC ${clip_SOURCE_DIR})
    set_lib_runtime_mt(clip)
endif()

# ----

# LIB ----
file(GLOB_RECURSE RAWRBOX_UI_IMPORTS "src/*.cpp" "include/*.hpp")
add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_UI_IMPORTS})
add_dependencies(${output_target} copy_resources_ui)
target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_UI)
target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC RAWRBOX.RENDER RAWRBOX.RESOURCES clip)

set_lib_runtime_mt(${output_target})
# --------------

# TEST ---- include(../cmake/catch2.cmake)
# --------------
