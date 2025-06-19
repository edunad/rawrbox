# Packages ----
if(RAWRBOX_BUILD_TESTING)
    message(STATUS "Enabled testing for ${output_target}")
    enable_testing()

    CPMAddPackage(
        NAME
            Catch2
        GITHUB_REPOSITORY
            catchorg/Catch2
        VERSION
            3.8.1
        OPTIONS
            "CATCH_INSTALL_DOCS OFF"
            "CATCH_CONFIG_FAST_COMPILE ON"
            "CATCH_INSTALL_EXTRAS ON")

    list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
    list(APPEND RAWRBOX_EXTRA_TEST_LIBS Catch2::Catch2WithMain)

    set_lib_runtime_mt(Catch2)
    set_lib_runtime_mt(Catch2WithMain)

    include(CTest)
    include(Catch)
endif()
# --------------

# TESTING ----
if(RAWRBOX_BUILD_TESTING)
    file(GLOB_RECURSE RAWRBOX_TESTS_IMPORTS "tests/*.spec.cpp")

    add_executable(${output_target}-TESTS ${RAWRBOX_TESTS_IMPORTS})
    target_compile_definitions(${output_target}-TESTS PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
    target_compile_definitions(${output_target}-TESTS PUBLIC RAWRBOX_TESTING)
    target_compile_features(${output_target}-TESTS PRIVATE cxx_std_${CMAKE_CXX_STANDARD})
    target_link_libraries(${output_target}-TESTS PRIVATE ${output_target} ${RAWRBOX_EXTRA_TEST_LIBS})

    set_lib_runtime_mt(${output_target}-TESTS)
    catch_discover_tests(${output_target}-TESTS DISCOVERY_MODE PRE_TEST)
endif()
# --------------
