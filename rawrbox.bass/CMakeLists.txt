# Project setup
project("RAWRBOX.BASS" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - BASS lib" LANGUAGES CXX)
set(output_target RAWRBOX.BASS)
# --------------

# Packages ----
set(BASS_VERSION 24)
find_package(BASS REQUIRED)
# --------------

# Grab source files
file(GLOB_RECURSE RAWRBOX_BASS_SOURCES "src/*.cpp" "include/*.hpp")

if(RAWRBOX_BUILD_RAWRBOX_RESOURCES)
    set(RAWRBOX_EXTRA_LIBS RAWRBOX.RESOURCES)
else()
    remove_folder("${RAWRBOX_BASS_SOURCES}" "/resources/")
    set(RAWRBOX_BASS_SOURCES "${CLEAN_SOURCES}")
endif()

if(RAWRBOX_BUILD_RAWRBOX_SCRIPTING)
    list(APPEND RAWRBOX_EXTRA_LIBS RAWRBOX.SCRIPTING)
    message(STATUS "Enabling RAWRBOX.BASS scripting plugin")
else()
    remove_folder("${RAWRBOX_BASS_SOURCES}" "/scripting/")
    set(RAWRBOX_BASS_SOURCES "${CLEAN_SOURCES}")
endif()

# ----

# Copy content - BASS
add_custom_target(
    copy_lib_bass ALL COMMAND ${CMAKE_COMMAND} -E copy ${BASS_BINARY} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "Copying BASS lib into binary directory")

add_custom_target(
    copy_lib_bass_fx ALL COMMAND ${CMAKE_COMMAND} -E copy ${BASS_BINARY_FX}
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "Copying BASS FX lib into binary directory")

set(RAWRBOX_EXTRA_DEPS copy_lib_bass copy_lib_bass_fx)
# ------------------

# Project setup
add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_BASS_SOURCES})

if(WIN32)
    add_dependencies(${output_target} ${RAWRBOX_EXTRA_DEPS})
endif()

copy_resources(TARGET ${output_target})
if(RAWRBOX_BUILD_RAWRBOX_SCRIPTING)
    copy_lua_libs(TARGET ${output_target})
endif()

target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_BASS)
target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})

target_link_libraries(${output_target} PUBLIC
    RAWRBOX.ENGINE
    RAWRBOX.MATH

    ${RAWRBOX_EXTRA_LIBS}

    BASS::BASS
    BASS::FX
)

set_lib_runtime_mt(${output_target})
