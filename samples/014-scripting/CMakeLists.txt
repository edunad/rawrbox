﻿project("014-scripting")

# Grab source files
file(GLOB_RECURSE RAWRBOX_SAMPLE_SOURCES "include/*.hpp" "src/*.cpp")

# Project setup
set(output_target 014-scripting)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE})

# Copy content - MODS
add_custom_target(
    link_mods_samples ALL COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/mods
                                  ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/mods
    COMMENT "Linking MODS resources into binary directory")
# ------------------

add_executable(${output_target} ${RAWRBOX_SAMPLE_SOURCES})
add_dependencies(${output_target} copy_resources_samples link_mods_samples)

target_compile_features(${output_target} PUBLIC cxx_std_20)
target_include_directories(${output_target} PRIVATE "include")
target_compile_features(${output_target} PRIVATE cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PRIVATE RAWRBOX.RENDER RAWRBOX.SCRIPTING)

set_lib_runtime_mt(${output_target})

if(NOT WIN32)
    set_target_properties(${output_target} PROPERTIES LINK_FLAGS -Wl,-rpath,'\$ORIGIN')
else()
    add_compile_options("/EHsc")
endif()