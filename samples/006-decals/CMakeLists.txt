set(output_target 006-decals)
project(${output_target})
# --------------

# Grab source files
file(GLOB_RECURSE RAWRBOX_SAMPLE_SOURCES "include/*.hpp" "src/*.cpp")

# Project setup
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE})

add_executable(${output_target} ${RAWRBOX_SAMPLE_SOURCES})
add_dependencies(${output_target} copy_resources_samples)
target_compile_features(${output_target} PUBLIC cxx_std_20)
target_include_directories(${output_target} PRIVATE "include")
target_compile_features(${output_target} PRIVATE cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PRIVATE RAWRBOX.RENDER_TEMP)

set_lib_runtime_mt(${output_target})

if(NOT WIN32)
    set_target_properties(${output_target} PROPERTIES LINK_FLAGS -Wl,-rpath,'\$ORIGIN')
else()
    add_compile_options("/EHsc")
endif()
