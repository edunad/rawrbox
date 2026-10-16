# Project setup
project("RAWRBOX.TOOLS" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - Tools" LANGUAGES CXX C)
set(output_target RAWRBOX.TOOLS)
# --------------

# LIBS ---
add_subdirectory(deps)
# ----

# LIB ----
file(GLOB_RECURSE RAWRBOX_TOOLS_IMPORTS "include/*.hpp" "src/*.cpp")
list(FILTER RAWRBOX_TOOLS_IMPORTS EXCLUDE REGEX ".*/src/vpk/main\\.cpp$")

add_library(${output_target} STATIC ${RAWRBOX_TOOLS_IMPORTS})
target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_link_libraries(${output_target} PUBLIC RAWRBOX.UTILS basisu_encoder gltfpack_lib webp)

if (TARGET RAWRBOX.SCRIPTING)
    target_link_libraries(${output_target} PUBLIC RAWRBOX.SCRIPTING) # Lua -> bytecode
    target_compile_definitions(${output_target} PRIVATE RAWRBOX_LUAU_VERSION="${CPM_PACKAGE_luau_VERSION}")
endif ()

set_lib_runtime_mt(${output_target})
# --------------

# VPK ----
add_executable(rawrbox_vpk "src/vpk/main.cpp")
target_compile_features(rawrbox_vpk PRIVATE cxx_std_${CMAKE_CXX_STANDARD})
target_compile_definitions(rawrbox_vpk PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_link_libraries(rawrbox_vpk PRIVATE ${output_target})

set_lib_runtime_mt(rawrbox_vpk)
# --------------

# TEST ----
include(../cmake/catch2.cmake)

if (RAWRBOX_BUILD_TESTING)
    set_target_properties(${output_target}-TESTS PROPERTIES EXCLUDE_FROM_ALL OFF)
endif ()
# --------------