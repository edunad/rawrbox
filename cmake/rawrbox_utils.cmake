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
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${COPY_RESOURCES_SRC_DIR}/content
                    ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/${RAWRBOX_CONTENT_FOLDER}
            COMMENT "Copying ${COPY_RESOURCES_TARGET}'s resources into binary directory")
    else()
        add_custom_target(
            copy_resources_${COPY_RESOURCES_TARGET} ALL
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${COPY_RESOURCES_SRC_DIR}/content
                    ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/${RAWRBOX_CONTENT_FOLDER}
            COMMENT "Copying ${COPY_RESOURCES_TARGET}'s resources into binary directory")
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
                                                        ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/lua
            COMMENT "Copying ${COPY_LUA_LIBS_TARGET}'s LUA libs into binary directory")
    else()
        add_custom_target(
            copy_lua_libs_${COPY_LUA_LIBS_TARGET} ALL COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${COPY_LUA_SRC_DIR}/lua
                                                        ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/lua
            COMMENT "Copying ${COPY_LUA_LIBS_TARGET}'s LUA libs into binary directory")
    endif()

    add_dependencies(${COPY_LUA_LIBS_TARGET} copy_lua_libs_${COPY_LUA_LIBS_TARGET})
endfunction()
