set(output_target 014-scripting)
project(${output_target})
# --------------

# Grab source files
file(GLOB_RECURSE RAWRBOX_SAMPLE_SOURCES "include/*.hpp" "src/*.cpp")

# Copy content - MODS
add_custom_target(
    link_mods_samples ALL COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/mods
                                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mods
    COMMENT "Linking MODS resources into binary directory")
# ------------------

if(RAWRBOX_BUILD_RAWRBOX_NETWORK)
    list(APPEND EXTRA_LIBS RAWRBOX.NETWORK)
endif()

if(RAWRBOX_BUILD_RAWRBOX_BASS)
    list(APPEND EXTRA_LIBS RAWRBOX.BASS)
endif()

if(RAWRBOX_BUILD_RAWRBOX_UI)
    list(APPEND EXTRA_LIBS RAWRBOX.UI)
endif()

add_executable(${output_target} ${RAWRBOX_SAMPLE_SOURCES})
add_dependencies(${output_target} link_mods_samples)
add_dependencies(${output_target} copy_resources_samples)

target_compile_features(${output_target} PUBLIC cxx_std_20)
target_include_directories(${output_target} PRIVATE "include")
target_compile_features(${output_target} PRIVATE cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PRIVATE RAWRBOX.RENDER RAWRBOX.SCRIPTING ${EXTRA_LIBS})

set_lib_runtime_mt(${output_target})
copy_required_dlls(${output_target}) # Required by diligent

if(NOT WIN32)
    set_target_properties(${output_target} PROPERTIES LINK_FLAGS -Wl,-rpath,'\$ORIGIN')
else()
    add_compile_options("/EHsc")
endif()
