
# Packages ----
if(RAWRBOX_BUILD_TESTING)
	message("-- Enabled testing for ${output_target}")
    enable_testing()

	find_package(Catch2 3 REQUIRED)
	list(APPEND RAWRBOX_EXTRA_TEST_LIBS Catch2::Catch2WithMain)

	if(RAWRBOX_ENABLE_DEBUG_SUPPORT)
        if(NOT output_target MATCHES "RAWRBOX.DEBUG")
		    list(APPEND RAWRBOX_EXTRA_TEST_LIBS RAWRBOX.DEBUG)
        endif()
	endif()

    include(CTest)
    include(Catch)
endif()
#--------------

# TESTING ----
if(RAWRBOX_BUILD_TESTING)
    file(GLOB_RECURSE RAWRBOX_TESTS_IMPORTS "tests/*.spec.cpp")

    add_executable(${output_target}-TESTS ${RAWRBOX_TESTS_IMPORTS})
    target_compile_features(${output_target}-TESTS PRIVATE cxx_std_${CMAKE_CXX_STANDARD})
    target_link_libraries(${output_target}-TESTS PRIVATE ${output_target} ${RAWRBOX_EXTRA_TEST_LIBS})

    catch_discover_tests(${output_target}-TESTS
        DISCOVERY_MODE PRE_TEST
    )
endif()
#--------------
