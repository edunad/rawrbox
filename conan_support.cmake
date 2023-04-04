
## Adapted from https://github.com/conan-io/cmake-conan/blob/develop2/conan_support.cmake
## hijacked by edunad

function(detect_os OS)
    # it could be cross compilation
    message(STATUS "Conan-cmake: cmake_system_name=${CMAKE_SYSTEM_NAME}")
    if(CMAKE_SYSTEM_NAME AND NOT CMAKE_SYSTEM_NAME STREQUAL "Generic")
        if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
            set(${OS} Macos CACHE STRING [PARENT_SCOPE])
        elseif(${CMAKE_SYSTEM_NAME} STREQUAL "QNX")
            set(${OS} Neutrino CACHE STRING [PARENT_SCOPE])
        else()
            set(${OS} ${CMAKE_SYSTEM_NAME} CACHE STRING [PARENT_SCOPE])
        endif()
    endif()
endfunction()


function(detect_cxx_standard CXX_STANDARD)
    set(${CXX_STANDARD} ${CMAKE_CXX_STANDARD} CACHE STRING [PARENT_SCOPE])
    if (CMAKE_CXX_EXTENSIONS)
        set(${CXX_STANDARD} "gnu${CMAKE_CXX_STANDARD}" CACHE STRING [PARENT_SCOPE])
    endif()
endfunction()


function(detect_compiler COMPILER COMPILER_VERSION)
    if(DEFINED CMAKE_CXX_COMPILER_ID)
        set(_COMPILER ${CMAKE_CXX_COMPILER_ID})
        set(_COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION})
    else()
        if(NOT DEFINED CMAKE_C_COMPILER_ID)
            message(FATAL_ERROR "C or C++ compiler not defined")
        endif()
        set(_COMPILER ${CMAKE_C_COMPILER_ID})
        set(_COMPILER_VERSION ${CMAKE_C_COMPILER_VERSION})
    endif()

    message(STATUS "Conan-cmake: CMake compiler=${_COMPILER}")
    message(STATUS "Conan-cmake: CMake cmpiler version=${_COMPILER_VERSION}")

    if(_COMPILER MATCHES MSVC)
        set(_COMPILER "msvc")
        string(SUBSTRING ${MSVC_VERSION} 0 3 _COMPILER_VERSION)
    elseif(_COMPILER MATCHES AppleClang)
        set(_COMPILER "apple-clang")
        string(REPLACE "." ";" VERSION_LIST ${CMAKE_CXX_COMPILER_VERSION})
        list(GET VERSION_LIST 0 _COMPILER_VERSION)
    elseif(_COMPILER MATCHES Clang)
        set(_COMPILER "clang")
        string(REPLACE "." ";" VERSION_LIST ${CMAKE_CXX_COMPILER_VERSION})
        list(GET VERSION_LIST 0 _COMPILER_VERSION)
    elseif(_COMPILER MATCHES GNU)
        set(_COMPILER "gcc")
        string(REPLACE "." ";" VERSION_LIST ${CMAKE_CXX_COMPILER_VERSION})
        list(GET VERSION_LIST 0 _COMPILER_VERSION)
    endif()

    message(STATUS "Conan-cmake: [settings] compiler=${_COMPILER}")
    message(STATUS "Conan-cmake: [settings] compiler.version=${_COMPILER_VERSION}")

    set(${COMPILER} ${_COMPILER} PARENT_SCOPE)
    set(${COMPILER_VERSION} ${_COMPILER_VERSION} PARENT_SCOPE)
endfunction()

function(detect_build_type BUILD_TYPE)
    if(NOT CMAKE_CONFIGURATION_TYPES)
        # Only set when we know we are in a single-configuration generator
        # Note: we may want to fail early if `CMAKE_BUILD_TYPE` is not defined
        set(${BUILD_TYPE} ${CMAKE_BUILD_TYPE} PARENT_SCOPE)
    endif()
endfunction()


function(detect_host_profile output_file)
    detect_os(MYOS)
    detect_compiler(MYCOMPILER MYCOMPILER_VERSION)
    detect_cxx_standard(MYCXX_STANDARD)
    detect_build_type(MYBUILD_TYPE)

    set(PROFILE "")
    string(APPEND PROFILE "include(default)\n")
    string(APPEND PROFILE "[settings]\n")
    if(MYOS)
        string(APPEND PROFILE os=${MYOS} "\n")
    endif()
    if(MYCOMPILER)
        string(APPEND PROFILE compiler=${MYCOMPILER} "\n")
    endif()
    if(MYCOMPILER_VERSION)
        string(APPEND PROFILE compiler.version=${MYCOMPILER_VERSION} "\n")
    endif()
    if(MYCXX_STANDARD)
        string(APPEND PROFILE compiler.cppstd=${MYCXX_STANDARD} "\n")
    endif()

	string(APPEND PROFILE compiler.runtime=dynamic "\n")

    if(MYBUILD_TYPE)
        string(APPEND PROFILE "build_type=${MYBUILD_TYPE}\n")
    endif()

    if(NOT DEFINED output_file)
        set(_FN "${CMAKE_BINARY_DIR}/profile")
    else()
        set(_FN ${output_file})
    endif()

    message(STATUS "Conan-cmake: Make Generator ${CMAKE_MAKE_PROGRAM}")
    message(STATUS "Conan-cmake: Generator ${CMAKE_GENERATOR}")

    string(APPEND PROFILE "[conf]\n")
    string(APPEND PROFILE "tools.cmake.cmaketoolchain:generator=${CMAKE_GENERATOR}\n")

    message(STATUS "Conan-cmake: Creating profile ${_FN}")
    file(WRITE ${_FN} ${PROFILE})
endfunction()


function(conan_profile_detect_default)
    message(STATUS "Conan-cmake: Checking if a default profile exists")
    execute_process(COMMAND conan profile path default
                    RESULT_VARIABLE return_code
                    OUTPUT_VARIABLE conan_stdout
                    ERROR_VARIABLE conan_stderr
                    ECHO_ERROR_VARIABLE    # show the text output regardless
                    ECHO_OUTPUT_VARIABLE
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    if(NOT ${return_code} EQUAL "0")
        message(STATUS "Conan-cmake: The default profile doesn't exist, detecting it.")
        execute_process(COMMAND conan profile detect
            RESULT_VARIABLE return_code
            OUTPUT_VARIABLE conan_stdout
            ERROR_VARIABLE conan_stderr
            ECHO_ERROR_VARIABLE    # show the text output regardless
            ECHO_OUTPUT_VARIABLE
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    endif()
endfunction()

function(conan_provide_dependency package_name)
	if(NOT CONAN_INSTALL_SUCCESS)
		message(STATUS "CMake-conan: Detecting default profile...")
		conan_profile_detect_default()

		message(STATUS "CMake-conan: Writting profile locally...")
		detect_host_profile(${CMAKE_BINARY_DIR}/conan_host_profile)

		message(STATUS "CMake-conan: Installing with ${CMAKE_BINARY_DIR}/conan_host_profile profile...")
		execute_process(COMMAND conan install . --output-folder=build --build=missing --profile ${CMAKE_BINARY_DIR}/conan_host_profile --format=json
			RESULT_VARIABLE conan_return_code
			OUTPUT_VARIABLE conan_stdout
			ERROR_VARIABLE conan_stderr
			ECHO_ERROR_VARIABLE
			#ECHO_OUTPUT_VARIABLE
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

		if(NOT "${conan_return_code}" STREQUAL "0")
			message(FATAL_ERROR "CMake-conan: Failed to install ${conan_return_code}")
		endif()

		string(JSON CONAN_GENERATORS_FOLDER GET ${conan_stdout} graph nodes 0 generators_folder)
        set(CONAN_GENERATORS_FOLDER "${CONAN_GENERATORS_FOLDER}" CACHE STRING [PARENT_SCOPE])

        message(STATUS "CMake-conan: CONAN_GENERATORS_FOLDER=${CONAN_GENERATORS_FOLDER}")
		message(STATUS "CMake-conan: Done!")

		file(REMOVE ${CMAKE_SOURCE_DIR}/CMakeUserPresets.json) # REEEE conan

        set(CONAN_INSTALL_SUCCESS TRUE CACHE BOOL "Conan install has been invoked and was successful")
	endif()

	find_package(${ARGN} BYPASS_PROVIDER)
endfunction()
