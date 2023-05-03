
# Project setup
project ("RAWRBOX.MATH" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - Math lib" LANGUAGES CXX)
set(output_target RAWRBOX.MATH)


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


# LIB ----
file(GLOB_RECURSE RAWRBOX_MATH_IMPORTS "include/*.h" "include/*.hpp" )
add_library(${output_target} INTERFACE ${RAWRBOX_MATH_IMPORTS})

target_include_directories(${output_target} INTERFACE "include")
#--------------

# LIB ----
if(RAWRBOX_BUILD_TESTING)
    file(GLOB_RECURSE RAWRBOX_MATH_TESTS_IMPORTS "tests/*.spec.cpp" )

    add_executable(${output_target}-TESTS ${RAWRBOX_MATH_TESTS_IMPORTS})
    target_compile_features(${output_target}-TESTS PRIVATE cxx_std_${CMAKE_CXX_STANDARD})
    target_link_libraries(${output_target}-TESTS PRIVATE ${output_target} Catch2::Catch2WithMain)

    catch_discover_tests(${output_target}-TESTS
        DISCOVERY_MODE PRE_TEST
    )
endif()
#--------------
