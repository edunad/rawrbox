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

function(copy_resources)
    set(oneValueArgs TARGET SRC_DIR)
    set(options "")
    set(multiValueArgs "")

    cmake_parse_arguments(COPY_RESOURCES "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT DEFINED COPY_RESOURCES_SRC_DIR)
        set(COPY_RESOURCES_SRC_DIR ${CMAKE_CURRENT_LIST_DIR})
    endif()

    if(${CMAKE_VERSION} VERSION_LESS "3.26.0")
        add_custom_target(
            copy_resources_${COPY_RESOURCES_TARGET} ALL
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${COPY_RESOURCES_SRC_DIR}/assets
                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${RAWRBOX_CONTENT_FOLDER}
            COMMENT "Copying ${COPY_RESOURCES_TARGET}'s assets into binary directory")
    else()
        add_custom_target(
            copy_resources_${COPY_RESOURCES_TARGET} ALL
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${COPY_RESOURCES_SRC_DIR}/assets
                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${RAWRBOX_CONTENT_FOLDER}
            COMMENT "Copying ${COPY_RESOURCES_TARGET}'s assets into binary directory")
    endif()

    add_dependencies(${COPY_RESOURCES_TARGET} copy_resources_${COPY_RESOURCES_TARGET})
endfunction()

function(copy_lua_libs)
    set(oneValueArgs TARGET SRC_DIR)
    set(options "")
    set(multiValueArgs "")

    cmake_parse_arguments(COPY_LUA_LIBS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT DEFINED COPY_LUA_LIBS_SRC_DIR)
        set(COPY_LUA_SRC_DIR ${CMAKE_CURRENT_LIST_DIR})
    endif()

    if(${CMAKE_VERSION} VERSION_LESS "3.26.0")
        add_custom_target(
            copy_lua_libs_${COPY_LUA_LIBS_TARGET} ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${COPY_LUA_SRC_DIR}/lua
                                                                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${RAWRBOX_CONTENT_FOLDER}/lua
            COMMENT "Copying ${COPY_LUA_LIBS_TARGET}'s LUA libs into binary directory")
    else()
        add_custom_target(
            copy_lua_libs_${COPY_LUA_LIBS_TARGET} ALL COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${COPY_LUA_SRC_DIR}/lua
                                                                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${RAWRBOX_CONTENT_FOLDER}/lua
            COMMENT "Copying ${COPY_LUA_LIBS_TARGET}'s LUA libs into binary directory")
    endif()

    add_dependencies(${COPY_LUA_LIBS_TARGET} copy_lua_libs_${COPY_LUA_LIBS_TARGET})
endfunction()
#
#function(mute_dependencies_warnings FILTER)
#    if(WIN32)
#        function (_get_all_cmake_targets out_var current_dir)
#            get_property(targets DIRECTORY ${current_dir} PROPERTY BUILDSYSTEM_TARGETS)
#            get_property(subdirs DIRECTORY ${current_dir} PROPERTY SUBDIRECTORIES)
#
#            foreach(subdir ${subdirs})
#                _get_all_cmake_targets(subdir_targets ${subdir})
#                list(APPEND targets ${subdir_targets})
#            endforeach()
#
#            set(${out_var} ${targets} PARENT_SCOPE)
#        endfunction()
#
#        _get_all_cmake_targets(all_targets ${CMAKE_CURRENT_LIST_DIR})
#        foreach(target ${all_targets})
#            get_target_property(target_type ${target} TYPE)
#
#            if(target MATCHES ${FILTER})
#                message(STATUS "Ignoring target ${target}")
#            else()
#                if (
#                    #target_type STREQUAL "EXECUTABLE" OR
#                    target_type STREQUAL "SHARED_LIBRARY" OR
#                    target_type STREQUAL "STATIC_LIBRARY")
#
#                    message(STATUS "Disabling warnings for ${target}: ${target_type}")
#                    target_compile_options(${target} PRIVATE
#                        /wd4100
#                        /wd4456
#                        /wd4458
#                        /wd4459
#                        /wd4189
#                        /wd4505
#                        /wd4324
#                        /wd4244
#                        /wd4701
#                        /wd4703
#                        /wd4127
#                        /wd4291
#                        /wd4201
#                        /wd4702
#                        /wd4706
#                        /wd4245
#                        /wd5054
#                        /wd4457
#                        /wd4389
#                        /wd4018
#                        /wd4267
#                    )
#                endif()
#            endif()
#
#        endforeach()
#    endif()
#endfunction()
#
