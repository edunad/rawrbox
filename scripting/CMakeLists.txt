# Project setup
project("RAWRBOX.SCRIPTING" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - SCRIPTING lib" LANGUAGES C CXX)
set(output_target RAWRBOX.SCRIPTING)

## LIBS ---
if(RAWRBOX_BUILD_RAWRBOX_SCRIPTING_USE_LUAJIT)
    message(STATUS "Enabled LuaJIT support")
    CPMAddPackage(
        NAME
        luajit
        GITHUB_REPOSITORY
        #9chu/LuaJIT-cmake
        edunad/LuaJIT-cmake
        GIT_TAG
        master
        OPTIONS
        "LUAJIT_DISABLE_BUFFER ON")

    if(luajit_ADDED)
        set_lib_runtime_mt(liblua-static)

        set(RAWRBOX_SCRIPTING_EXTRA_LIB liblua-static)
        set(SOL2_LUA_VERSION "LuaJIT" CACHE STRING "" FORCE)
    endif()
else()
    CPMAddPackage(
        NAME
        lua
        GITHUB_REPOSITORY
        lua/lua
        GIT_TAG
        v5.4.6
        DOWNLOAD_ONLY
        YES)

    if(lua_ADDED)
        file(GLOB lua_sources ${lua_SOURCE_DIR}/*.c)
        list(REMOVE_ITEM lua_sources "${lua_SOURCE_DIR}/lua.c" "${lua_SOURCE_DIR}/luac.c")

        add_library(lua STATIC ${lua_sources})
        target_include_directories(lua SYSTEM PUBLIC $<BUILD_INTERFACE:${lua_SOURCE_DIR}>)

        set_lib_runtime_mt(lua)
    endif()

    set(RAWRBOX_SCRIPTING_EXTRA_LIB lua)
    set(RAWRBOX_SCRIPTING_EXTRA_DEFS SOL_LUAJIT=1)
endif()

# SOL -----------
CPMAddPackage(NAME sol2 GITHUB_REPOSITORY ThePhD/sol2 VERSION 3.3.0)

if(sol2_ADDED)
    set_lib_runtime_mt(sol2)
endif()
# --------------

# Copy content - lua
add_custom_target(
    copy_resources_lua ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/lua
                                   ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/lua
    COMMENT "Copying LUA resources into binary directory")
# ------------------

file(GLOB_RECURSE RAWRBOX_SCRIPTING_IMPORTS "src/*.cpp" "include/*.hpp")

add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_SCRIPTING_IMPORTS})
add_dependencies(${output_target} copy_resources_lua)

target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_SCRIPTING SOL_ALL_SAFETIES_ON=1 ${RAWRBOX_SCRIPTING_EXTRA_DEFS})
target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC RAWRBOX.ENGINE RAWRBOX.MATH ${RAWRBOX_SCRIPTING_EXTRA_LIB} sol2::sol2)

set_lib_runtime_mt(${output_target})
# --------------

# TEST ----
include(../cmake/catch2.cmake)
# --------------
