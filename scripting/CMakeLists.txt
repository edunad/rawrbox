# Project setup
project("RAWRBOX.SCRIPTING" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - SCRIPTING lib" LANGUAGES C CXX)
set(output_target RAWRBOX.SCRIPTING)
# --------------

# LIBS ---
if(RAWRBOX_BUILD_RAWRBOX_SCRIPTING_USE_LUAJIT)
    message(STATUS "Enabled LuaJIT support")
    message(WARNING "LuaJIT should not be used right now, check issue https://github.com/edunad/rawrbox/issues/73")

    #CPMAddPackage(NAME luajit GITHUB_REPOSITORY WohlSoft/LuaJIT GIT_TAG v2.1)
    #if(luajit_ADDED)
    #    set_lib_runtime_mt(luajit)
    #    set_lib_runtime_mt(libluajit)
    #    list(APPEND RAWRBOX_SCRIPTING_EXTRA_LIB libluajit)
    #    set(SOL2_LUA_VERSION "LuaJIT" CACHE STRING "" FORCE)
    #endif()

    CPMAddPackage(
        NAME
        luajit
        GITHUB_REPOSITORY
        edunad/LuaJIT-cmake
        GIT_TAG
        master
        OPTIONS
        "LUAJIT_DISABLE_BUFFER ON"
        "LUA_USE_APICHECK ON")

    if(luajit_ADDED)
        set_lib_runtime_mt(liblua-static)

        set(RAWRBOX_SCRIPTING_EXTRA_LIB liblua-static)
        set(SOL2_LUA_VERSION "LuaJIT" CACHE STRING "" FORCE)
    endif()
else()
    message(STATUS "LuaJIT support disabled, using normal LUA")
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
        FILE(GLOB lua_sources ${lua_SOURCE_DIR}/*.c)
        list(REMOVE_ITEM lua_sources "${lua_SOURCE_DIR}/lua.c" "${lua_SOURCE_DIR}/luac.c" "${lua_SOURCE_DIR}/onelua.c")
        add_library(lua STATIC ${lua_sources})

        target_include_directories(lua PUBLIC $<BUILD_INTERFACE:${lua_SOURCE_DIR}>)
        set_lib_runtime_mt(lua)
    endif()

    list(APPEND RAWRBOX_SCRIPTING_EXTRA_LIB lua)
endif()

# SOL -----------
CPMAddPackage(NAME sol2 GITHUB_REPOSITORY ThePhD/sol2 VERSION 3.3.0)

if(sol2_ADDED)
    set_lib_runtime_mt(sol2)
endif()
# --------------

file(GLOB_RECURSE RAWRBOX_SCRIPTING_IMPORTS "src/*.cpp" "include/*.hpp")

add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_SCRIPTING_IMPORTS})
copy_lua_libs(TARGET ${output_target})
target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_SCRIPTING SOL_ALL_SAFETIES_ON=1 SOL_EXCEPTIONS_SAFE_PROPAGATION=1)
if(RAWRBOX_BUILD_RAWRBOX_SCRIPTING_USE_LUAJIT)
    target_compile_definitions(${output_target} PUBLIC RAWRBOX_SCRIPTING_LUAJIT SOL_LUAJIT=1)
endif()

target_include_directories(${output_target} PUBLIC "include" ${luajit_SOURCE_DIR}/src)
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC RAWRBOX.ENGINE RAWRBOX.MATH ${RAWRBOX_SCRIPTING_EXTRA_LIB} sol2::sol2 nlohmann_json)

set_lib_runtime_mt(${output_target})
# --------------

# TEST ----
include(../cmake/catch2.cmake)
# --------------
