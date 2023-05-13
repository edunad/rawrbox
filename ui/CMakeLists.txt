
# Project setup
project ("RAWRBOX.UI" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - UI lib" LANGUAGES CXX)
set(output_target RAWRBOX.UI)

# LIB ----
file(GLOB_RECURSE RAWRBOX_UI_IMPORTS "src/*.cpp" "include/*.hpp")
add_library(${output_target} ${RAWRBOX_UI_IMPORTS})
target_compile_definitions(${output_target} PUBLIC _CRT_SECURE_NO_WARNINGS NOMINMAX RAWRBOX_UI)
target_include_directories(${output_target} PUBLIC "include" )
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC RAWRBOX.RENDER)
#--------------

# TEST ----
#include(../cmake/catch2.cmake)
#--------------

# Copy content
add_custom_command( TARGET ${output_target} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/${RAWRBOX_CONTENT_FOLDER} ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/${RAWRBOX_CONTENT_FOLDER} )
# ------------------
