
# Project setup
project ("RAWRBOX.DEBUG" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - Debug util lib" LANGUAGES CXX)
set(output_target RAWRBOX.DEBUG)
#--------------

if(RAWRBOX_ENABLE_BASS_SUPPORT)
    set(RAWRBOX_DEBUG_EXTRA_LIBS RAWRBOX.BASS)
endif()

# Copy content - DEBUG
add_custom_target(copy_resources_debug ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_SOURCE_DIR}/${RAWRBOX_CONTENT_FOLDER}
        ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/${RAWRBOX_CONTENT_FOLDER}
        COMMENT "Copying DEBUG resources into binary directory")
# ------------------


# LIB ----
file(GLOB_RECURSE RAWRBOX_DEBUG_IMPORTS "include/*.hpp" "src/*.cpp" )

add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_DEBUG_IMPORTS})
add_dependencies(${output_target} copy_resources_debug)
target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX INTERFACE RAWRBOX_DEBUG)
target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC RAWRBOX.RENDER ${RAWRBOX_DEBUG_EXTRA_LIBS})
#--------------


# TEST ----
include(../cmake/catch2.cmake)
#--------------
