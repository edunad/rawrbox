
function(set_lib_runtime_mt target)
    if(RAWRBOX_BUILD_MSVC_MULTITHREADED_RUNTIME)
        message(STATUS "-- Setting ${target}@'s MSVC_RUNTIME_LIBRARY as MultiThreaded")

        if (("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows" OR "${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore") AND NOT MINGW)
            set_target_properties(${target} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        endif()
    endif()
endfunction()
