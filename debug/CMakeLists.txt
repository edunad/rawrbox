
# Project setup
project ("RAWRBOX.DEBUG" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - Debug util lib" LANGUAGES CXX)
set(output_target RAWRBOX.DEBUG)


# Packages ----
if(RAWRBOX_BUILD_TESTING)
	message("-- Enabled testing")
    enable_testing()

	find_package(Catch2 3 REQUIRED)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

    include(CTest)
    include(Catch)
endif()

#--------------

if(RAWRBOX_ENABLE_BASS_SUPPORT)
    set(RAWRBOX_DEBUG_DEFINES RAWRBOX_BASS)
    set(RAWRBOX_EXTRA_LIBS RAWRBOX.BASS)
endif()


# LIB ----
file(GLOB_RECURSE RAWRBOX_DEBUG_IMPORTS "include/*.hpp" "src/*.cpp" )
add_library(${output_target} ${RAWRBOX_DEBUG_IMPORTS})
target_compile_definitions(${output_target} PUBLIC _CRT_SECURE_NO_WARNINGS NOMINMAX ${RAWRBOX_DEBUG_DEFINES})
target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC ${RAWRBOX_SHADERS_TARGET_NAME}
	RAWRBOX.MATH
    RAWRBOX.RENDER
	RAWRBOX.UTILS

	${RAWRBOX_EXTRA_LIBS}
)
#--------------

# Copy content
add_custom_command( TARGET ${output_target} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/${RAWRBOX_CONTENT_FOLDER} ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/${RAWRBOX_CONTENT_FOLDER} )
# ------------------

# TESTING ----
if(RAWRBOX_BUILD_TESTING)
    file(GLOB_RECURSE RAWRBOX_DEBUG_TESTS_IMPORTS "tests/*.spec.cpp" )

    add_executable(${output_target}-TESTS ${RAWRBOX_DEBUG_TESTS_IMPORTS})
    target_compile_features(${output_target}-TESTS PRIVATE cxx_std_${CMAKE_CXX_STANDARD})
    target_link_libraries(${output_target}-TESTS PRIVATE ${output_target} Catch2::Catch2WithMain)

    catch_discover_tests(${output_target}-TESTS
        DISCOVERY_MODE PRE_TEST
    )
endif()
#--------------


