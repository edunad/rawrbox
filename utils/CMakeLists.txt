
# Project setup
set(output_target RAWRBOX.UTILS)

file(GLOB RAWRBOX_UTILS_IMPORTS "include/*.h" "include/*.hpp" )
add_library(${output_target} INTERFACE ${RAWRBOX_UTILS_IMPORTS})
target_include_directories(${output_target} INTERFACE "include")
