# Project setup
project("RAWRBOX.ENGINE" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - Engine lib" LANGUAGES CXX)
set(output_target RAWRBOX.ENGINE)

# DEPS ----
CPMAddPackage("gh:jeremy-rifkin/cpptrace@0.3.1")
if(cpptrace_ADDED)
  set_lib_runtime_mt(cpptrace-lib)
endif()
# --------------

# LIB ----
file(GLOB_RECURSE RAWRBOX_ENGINE_IMPORTS "src/*.cpp" "include/*.hpp")
add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_ENGINE_IMPORTS})
target_include_directories(${output_target} PUBLIC "include")
target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_ENGINE)
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC RAWRBOX.UTILS cpptrace::cpptrace)

set_lib_runtime_mt(${output_target})
# --------------

if(WIN32)
  add_custom_command(
    TARGET ${output_target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:cpptrace::cpptrace>
    $<TARGET_FILE_DIR:${output_target}>
  )
endif()

# TEST ----
include(../cmake/catch2.cmake)
# --------------
