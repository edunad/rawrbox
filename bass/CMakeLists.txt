
# Project setup
project ("RAWRBOX.BASS" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - BASS lib" LANGUAGES CXX)
set(output_target RAWRBOX.BASS)
set(BASS_VERSION 24)
#--------------

# Packages ----
find_package(BASS REQUIRED)

file(COPY "${BASS_BINARY}" DESTINATION "${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/")
file(COPY "${BASS_BINARY_FX}" DESTINATION "${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/")
#--------------

# Grab source files
file(GLOB_RECURSE RAWRBOX_BASS_SOURCES "src/*.cpp" "include/*.hpp")
# -----------------

if(RAWRBOX_BUILD_DEBUG)
    set(RAWRBOX_BASS_DEFINES RAWRBOX_DEBUG)
    list(APPEND RAWRBOX_EXTRA_LIBS RAWRBOX.DEBUG)
endif()

# Project setup
add_library(${output_target} ${RAWRBOX_BASS_SOURCES})
target_compile_definitions(${output_target} PUBLIC _CRT_SECURE_NO_WARNINGS NOMINMAX ${RAWRBOX_BASS_DEFINES})
target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC
	RAWRBOX.ENGINE
	RAWRBOX.MATH
	${RAWRBOX_EXTRA_LIBS}

	fmt::fmt

	BASS::BASS
	BASS::FX
)

# TEST ----
include(../cmake/catch2.cmake)
#--------------
