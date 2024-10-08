# Project setup
project("RAWRBOX.RESOURCES" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - Resources manager lib" LANGUAGES CXX)
set(output_target RAWRBOX.RESOURCES)

# LIB ----
file(GLOB_RECURSE RAWRBOX_RESOURCES_IMPORTS "src/*.cpp" "include/*.hpp")
if(RAWRBOX_BUILD_RAWRBOX_SCRIPTING)
    list(APPEND RAWRBOX_EXTRA_LIBS RAWRBOX.SCRIPTING)
    message(STATUS "Enabling RAWRBOX.RESOURCES scripting plugin")
else()
    remove_folder("${RAWRBOX_RESOURCES_IMPORTS}" "/scripting/")
    set(RAWRBOX_RESOURCES_IMPORTS "${CLEAN_SOURCES}")
endif()

add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_RESOURCES_IMPORTS})

target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_RESOURCES)
target_link_libraries(${output_target} PUBLIC RAWRBOX.UTILS ${RAWRBOX_EXTRA_LIBS})


set_lib_runtime_mt(${output_target})
# --------------

# TEST ----
include(../cmake/catch2.cmake)
# --------------
