# Project setup
project(
    "RAWRBOX.BASS"
    VERSION ${RAWRBOX_VERSION}
    DESCRIPTION "RawrBox - BASS lib"
    LANGUAGES CXX)
set(output_target RAWRBOX.BASS)
set(BASS_VERSION 24)
# --------------

# Packages ----
find_package(BASS REQUIRED)
# --------------

# Grab source files
file(GLOB_RECURSE RAWRBOX_BASS_SOURCES "src/*.cpp" "include/*.hpp")
# -----------------

if(RAWRBOX_BUILD_RESOURCES_MANAGER)
    list(APPEND RAWRBOX_EXTRA_LIBS RAWRBOX.RESOURCES)
endif()

# Copy content - BASS
add_custom_target(
    copy_resources_bass ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/${RAWRBOX_CONTENT_FOLDER}
            ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/${RAWRBOX_CONTENT_FOLDER}
    COMMENT "Copying BASS resources into binary directory")

add_custom_target(
    copy_dll_bass ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${BASS_BINARY} ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}
    COMMENT "Copying BASS DLL into binary directory")

add_custom_target(
    copy_dll_bass_fx ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${BASS_BINARY_FX} ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}
    COMMENT "Copying BASS FX DLL into binary directory")
# ------------------

# Project setup
add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_BASS_SOURCES})
add_dependencies(${output_target} copy_dll_bass copy_dll_bass_fx copy_resources_bass)
target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_BASS)
target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC RAWRBOX.ENGINE RAWRBOX.MATH ${RAWRBOX_EXTRA_LIBS} fmt::fmt BASS::BASS BASS::FX)

set_lib_runtime_mt(${output_target})
