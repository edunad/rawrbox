
# Grab source files
file(GLOB RAWRBOX_UTILS_IMPORTS "src/*.cpp" "src/*.c" "include/*.h" "include/*.hpp" )

# Project setup
set(output_target RAWRBOX.UTILS)
add_library(${output_target} ${RAWRBOX_UTILS_IMPORTS})

target_include_directories(${output_target} PUBLIC "include" )
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
