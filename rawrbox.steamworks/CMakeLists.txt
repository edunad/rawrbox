# Project setup
project("RAWRBOX.STEAMWORKS" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - STEAMWORKS lib" LANGUAGES C CXX)
set(output_target RAWRBOX.STEAMWORKS)
# --------------

# LIBS ---
set(STEAMWORKS_SDKPATH "${CMAKE_CURRENT_LIST_DIR}/_deps/steamworks")
find_package(STEAMWORKS REQUIRED)
# --------------

file(GLOB_RECURSE RAWRBOX_STEAMWORKS_IMPORTS "src/*.cpp" "include/*.hpp")

add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_STEAMWORKS_IMPORTS})

target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_compile_definitions(${output_target} PUBLIC RAWRBOX_STEAMWORKS STEAMWORKS_APPID=${STEAMWORKS_APPID})

target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target}
    PUBLIC
        RAWRBOX.UTILS
        STEAMWORKS::STEAMWORKS
        STEAMWORKS::APPTICKET
)

set_lib_runtime_mt(${output_target})
# --------------

# TEST ----
# include(../cmake/catch2.cmake)
# --------------
