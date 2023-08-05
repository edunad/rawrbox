# Project setup
project("RAWRBOX.SCRIPTING" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - SCRIPTING lib" LANGUAGES C CXX)
set(output_target RAWRBOX.SCRIPTING)

# LIBS ---
CPMAddPackage(
    NAME
    lua
    GIT_REPOSITORY
    https://github.com/lua/lua.git
    VERSION
    5.3.5
    DOWNLOAD_ONLY
    YES)

if(lua_ADDED)
    file(GLOB lua_sources ${lua_SOURCE_DIR}/*.c)
    list(REMOVE_ITEM lua_sources "${lua_SOURCE_DIR}/lua.c" "${lua_SOURCE_DIR}/luac.c")

    add_library(lua STATIC ${lua_sources})
    target_include_directories(lua SYSTEM PUBLIC $<BUILD_INTERFACE:${lua_SOURCE_DIR}>)
endif()

CPMAddPackage("gh:ThePhD/sol2#develop")

set_lib_runtime_mt(lua)
set_lib_runtime_mt(sol2)
# --------------

# Copy content - lua
add_custom_target(
    copy_resources_lua ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/lua
                                   ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/${RAWRBOX_CONTENT_FOLDER}
    COMMENT "Copying LUA resources into binary directory")
# ------------------

file(GLOB_RECURSE RAWRBOX_SCRIPTING_IMPORTS "src/*.cpp" "include/*.hpp")

add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_SCRIPTING_IMPORTS})
add_dependencies(${output_target} copy_resources_lua)

target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_SCRIPTING)
target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC RAWRBOX.ENGINE RAWRBOX.MATH sol2 lua)

set_lib_runtime_mt(${output_target})
# --------------

# TEST ----
#include(../cmake/catch2.cmake)
# --------------
