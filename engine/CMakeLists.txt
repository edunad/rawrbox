
# Project setup
project ("RAWRBOX.ENGINE" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - Engine lib" LANGUAGES CXX)
set(output_target RAWRBOX.ENGINE)

# LIB ----
file(GLOB_RECURSE RAWRBOX_ENGINE_IMPORTS "src/*.cpp" "include/*.hpp")
add_library(${output_target} ${RAWRBOX_ENGINE_IMPORTS})
target_include_directories(${output_target} PUBLIC "include" )
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC
	RAWRBOX.UTILS
)
#--------------

# TEST ----
include(../cmake/catch2.cmake)
#--------------
