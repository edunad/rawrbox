function(set_lib_runtime_mt target)
    if(RAWRBOX_BUILD_MSVC_MULTITHREADED_RUNTIME)
        message(STATUS "Setting ${target}'s MSVC_RUNTIME_LIBRARY as MultiThreaded")

        if(("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows" OR "${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore") AND NOT MINGW)
            set_target_properties(${target} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        endif()
    endif()
endfunction()

# From : https://stackoverflow.com/questions/24491129/excluding-directory-somewhere-in-file-structure-from-cmake-sourcefile-list
function(remove_folder glob_sources folderName)
    message(STATUS "Removing ${folderName} from sources")

    foreach(ITR ${glob_sources})
        if("${ITR}" MATCHES "(.*)${folderName}(.*)")
            list(REMOVE_ITEM glob_sources ${ITR})
        endif("${ITR}" MATCHES "(.*)${folderName}(.*)")
    endforeach(ITR)

    set(CLEAN_SOURCES "${glob_sources}" PARENT_SCOPE)
endfunction()
