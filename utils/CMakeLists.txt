
# Project setup
project ("RAWRBOX.UTILS" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - Utils lib" LANGUAGES CXX)
set(output_target RAWRBOX.UTILS)


# Grab source files
file(GLOB_RECURSE RAWRBOX_UTILS_IMPORTS "src/*.cpp" "src/*.c" "include/*.h" "include/*.hpp" )

if(NOT RAWRBOX_ENABLE_QHULL)
	list(REMOVE_ITEM RAWRBOX_UTILS_IMPORTS "include/rawrbox/utils/qhull.hpp")
else()
	find_package(Qhull REQUIRED)
	set(EXTRA_UTIL_LIBS Qhull::qhullstatic_r)
endif()

# TEST ----
if(RAWRBOX_BUILD_TESTING)
	message("-- Enabled testing")
    enable_testing()

	find_package(Catch2 3 REQUIRED)
	list(APPEND EXTRA_UTIL_LIBS Catch2::Catch2WithMain)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

    include(CTest)
    include(Catch)
endif()
#--------------

add_library(${output_target} ${RAWRBOX_UTILS_IMPORTS})
target_include_directories(${output_target} PUBLIC "include" )
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC
	RAWRBOX.MATH
	${EXTRA_UTIL_LIBS}
)

# TESTING ----
if(RAWRBOX_BUILD_TESTING)
    file(GLOB_RECURSE RAWRBOX_UTILS_TESTS_IMPORTS "tests/*.spec.cpp" )

    add_executable(${output_target}-TESTS ${RAWRBOX_UTILS_TESTS_IMPORTS})
    target_compile_features(${output_target}-TESTS PRIVATE cxx_std_${CMAKE_CXX_STANDARD})
    target_link_libraries(${output_target}-TESTS PRIVATE ${output_target} Catch2::Catch2WithMain)

    catch_discover_tests(${output_target}-TESTS
        DISCOVERY_MODE PRE_TEST
    )
endif()
#--------------
