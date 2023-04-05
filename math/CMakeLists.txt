
# Project setup
set(output_target RAWRBOX.MATH)

file(GLOB RAWRBOX_MATH_IMPORTS "include/*.h" "include/*.hpp" )
add_library(${output_target} INTERFACE ${RAWRBOX_MATH_IMPORTS})
target_include_directories(${output_target} INTERFACE "include")
