project ("RAWRBOX.BASS" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - BASS lib" LANGUAGES CXX)
set(output_target RAWRBOX.BASS)
set(BASS_VERSION 24)

# TEST ----
if(RAWRBOX_BUILD_TESTING)
	message("-- Enabled testing")
    enable_testing()

	find_package(Catch2 3 REQUIRED)
	list(APPEND RAWRBOX_EXTRA_LIBS Catch2::Catch2WithMain)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

    include(CTest)
    include(Catch)
endif()
#--------------

# Packages ----
find_package(BASS REQUIRED)

file(COPY "${BASS_BINARY}" DESTINATION "${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/")
file(COPY "${BASS_BINARY_FX}" DESTINATION "${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/")
#--------------

# Grab source files
file(GLOB_RECURSE RAWRBOX_BASS_SOURCES "src/*.cpp" "src/*.c" "include/*.h" "include/*.hpp")
# -----------------

# Project setup
add_library(${output_target} ${RAWRBOX_BASS_SOURCES})
target_compile_definitions(${output_target} PUBLIC _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC
	RAWRBOX.UTILS
	RAWRBOX.ENGINE
	RAWRBOX.DEBUG

	fmt::fmt

	BASS::BASS
	BASS::FX
	${RAWRBOX_EXTRA_LIBS}
)

# TESTING ----
if(RAWRBOX_BUILD_TESTING)
    file(GLOB_RECURSE RAWRBOX_BASS_TESTS_IMPORTS "tests/*.spec.cpp" )

    add_executable(${output_target}-TESTS ${RAWRBOX_BASS_TESTS_IMPORTS})
    target_compile_features(${output_target}-TESTS PRIVATE cxx_std_${CMAKE_CXX_STANDARD})
    target_link_libraries(${output_target}-TESTS PRIVATE ${output_target} Catch2::Catch2WithMain)

    catch_discover_tests(${output_target}-TESTS
        DISCOVERY_MODE PRE_TEST
    )
endif()
#--------------
