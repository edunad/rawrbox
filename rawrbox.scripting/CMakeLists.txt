# Project setup
project("RAWRBOX.SCRIPTING" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - SCRIPTING lib" LANGUAGES C CXX)
set(output_target RAWRBOX.SCRIPTING)
# --------------

# LIBS ---
CPMAddPackage(
    NAME
        LuaBridge3
    GITHUB_REPOSITORY
        kunitoki/LuaBridge3
    GIT_TAG
        master
    OPTIONS
        "LUABRIDGE_TESTING OFF"
        "LUABRIDGE_COVERAGE OFF"
    )

if(LuaBridge3_ADDED)
    set_lib_runtime_mt(LuaBridge)
endif()

CPMAddPackage(
    NAME
        luau
    GITHUB_REPOSITORY
        luau-lang/luau
    GIT_TAG
        0.637
    OPTIONS
        "LUAU_BUILD_CLI OFF"
        "LUAU_BUILD_TESTS OFF"
        "LUAU_BUILD_BENCHMARKS OFF"
        "LUAU_BUILD_WEB OFF"
    )

if(luau_ADDED)
    set_lib_runtime_mt(Luau.VM)
    set_lib_runtime_mt(Luau.Common)
    set_lib_runtime_mt(Luau.Compiler)
    set_lib_runtime_mt(Luau.Ast)
endif()
# --------------

file(GLOB_RECURSE RAWRBOX_SCRIPTING_IMPORTS "src/*.cpp" "include/*.hpp")

add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_SCRIPTING_IMPORTS})
copy_lua_libs(TARGET ${output_target})

target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_SCRIPTING RAWRBOX_SCRIPTING_UNSAFE=${RAWRBOX_BUILD_RAWRBOX_SCRIPTING_UNSAFE} RAWRBOX_SCRIPTING_EXCEPTION=${RAWRBOX_BUILD_RAWRBOX_SCRIPTING_EXCEPTION} LUAU=1)

target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target}
    PUBLIC
        RAWRBOX.ENGINE
        RAWRBOX.MATH

        # LUAU ----
        Luau.VM
        Luau.Common
        Luau.Compiler
        Luau.Analysis
        # ---------

        # LuaBridge ----
        LuaBridge
        # ---------
)

set_lib_runtime_mt(${output_target})
# --------------

# TEST ----
include(../cmake/catch2.cmake)
# --------------
