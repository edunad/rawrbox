

# Project setup
set(output_target RAWRBOX.MATH)

file(GLOB RAWRBOX_MATH_IMPORTS "include/*.h" "include/*.hpp" )
add_library(${output_target} INTERFACE ${RAWRBOX_MATH_IMPORTS})
target_include_directories(${output_target} INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${output_target}>
  $<INSTALL_INTERFACE:include/${output_target}>  # <prefix>/include/mylib
)

set_target_properties(
	${output_target}

	PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	IMPORTED_NO_SONAME TRUE
)
