
# Grab source files
file(GLOB_RECURSE RAWRBOX_UTILS_IMPORTS "src/*.cpp" "src/*.c" "include/*.h" "include/*.hpp" )

if(NOT RAWRBOX_BUILD_UTIL_QHULL)
	list(REMOVE_ITEM RAWRBOX_UTILS_IMPORTS "include/rawrbox/utils/qhull.hpp")
else()
	find_package(Qhull REQUIRED)
	set(EXTRA_UTIL_LIBS Qhull::qhullstatic_r)
endif()

# Project setup
set(output_target RAWRBOX.UTILS)
add_library(${output_target} ${RAWRBOX_UTILS_IMPORTS})

target_include_directories(${output_target} PUBLIC "include" )
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC
	RAWRBOX.MATH
	${EXTRA_UTIL_LIBS}
)
