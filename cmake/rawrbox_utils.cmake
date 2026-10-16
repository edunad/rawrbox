function(set_lib_runtime_mt target)
    if (RAWRBOX_BUILD_MSVC_MULTITHREADED_RUNTIME)
        message(STATUS "Setting ${target}'s MSVC_RUNTIME_LIBRARY as MultiThreaded")

        if (("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows" OR "${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore") AND NOT MINGW)
            get_target_property(aliased ${target} ALIASED_TARGET)
            if (aliased)
                set(target ${aliased})
            endif ()

            set_target_properties(${target} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        endif ()
    endif ()
endfunction()

# From : https://stackoverflow.com/questions/24491129/excluding-directory-somewhere-in-file-structure-from-cmake-sourcefile-list
function(remove_folder glob_sources folderName)
    message(STATUS "Removing ${folderName} from sources")

    foreach (ITR ${glob_sources})
        if ("${ITR}" MATCHES "(.*)${folderName}(.*)")
            list(REMOVE_ITEM glob_sources ${ITR})
        endif ("${ITR}" MATCHES "(.*)${folderName}(.*)")
    endforeach (ITR)

    set(CLEAN_SOURCES "${glob_sources}" PARENT_SCOPE)
endfunction()

function(add_resources) # Adds resources to the VPK packing
    set(oneValueArgs SRC_DIR FOLDER PREFIX)
    set(options "")
    set(multiValueArgs PRELOAD RAW DATA)

    cmake_parse_arguments(ADD_RESOURCES "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT DEFINED ADD_RESOURCES_SRC_DIR)
        set(ADD_RESOURCES_SRC_DIR ${CMAKE_CURRENT_LIST_DIR})
    endif ()

    if (NOT DEFINED ADD_RESOURCES_FOLDER)
        message(FATAL_ERROR "add_resources requires a FOLDER")
    endif ()

    if (NOT DEFINED ADD_RESOURCES_PREFIX)
        if (ADD_RESOURCES_FOLDER STREQUAL "assets")
            set(ADD_RESOURCES_PREFIX ${RAWRBOX_CONTENT_FOLDER})
        else ()
            set(ADD_RESOURCES_PREFIX ${ADD_RESOURCES_FOLDER})
        endif ()
    endif ()

    set(RESOURCES_FOLDER "${ADD_RESOURCES_SRC_DIR}/${ADD_RESOURCES_FOLDER}")
    if (NOT EXISTS "${RESOURCES_FOLDER}")
        message(FATAL_ERROR "Resources folder '${RESOURCES_FOLDER}' not found")
    endif ()

    set_property(GLOBAL APPEND PROPERTY RAWRBOX_PACK_INPUTS "${ADD_RESOURCES_PREFIX}=${RESOURCES_FOLDER}")

    foreach (OPTION PRELOAD RAW DATA)
        foreach (GLOB ${ADD_RESOURCES_${OPTION}})
            if (ADD_RESOURCES_PREFIX)
                set_property(GLOBAL APPEND PROPERTY RAWRBOX_PACK_${OPTION} ${ADD_RESOURCES_PREFIX}/${GLOB})
            else ()
                set_property(GLOBAL APPEND PROPERTY RAWRBOX_PACK_${OPTION} ${GLOB})
            endif ()
        endforeach ()
    endforeach ()
endfunction()

function(pack_assets)
    set(oneValueArgs TARGET NAME)
    set(options COMPRESS)
    set(multiValueArgs "")

    cmake_parse_arguments(PACK_ASSETS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT TARGET rawrbox_vpk)
        message(FATAL_ERROR "pack_assets requires the rawrbox_vpk tool")
    endif ()

    if (NOT DEFINED PACK_ASSETS_NAME)
        set(PACK_ASSETS_NAME pak01)
    endif ()

    # INPUTS ---
    get_property(PACK_INPUTS GLOBAL PROPERTY RAWRBOX_PACK_INPUTS)
    if (NOT PACK_INPUTS)
        message(FATAL_ERROR "pack_assets is missing resources! call add_resources first")
    endif ()

    set(PACK_ARGS "")
    set(PACK_DEPENDS "")

    foreach (INPUT ${PACK_INPUTS})
        list(APPEND PACK_ARGS --input ${INPUT})

        string(FIND "${INPUT}" "=" INPUT_SPLIT)
        math(EXPR INPUT_SPLIT "${INPUT_SPLIT} + 1")
        string(SUBSTRING "${INPUT}" ${INPUT_SPLIT} -1 INPUT_FOLDER)

        file(GLOB_RECURSE INPUT_FILES CONFIGURE_DEPENDS "${INPUT_FOLDER}/*")
        list(APPEND PACK_DEPENDS ${INPUT_FILES})
    endforeach ()
    # ----------

    # OPTIONS ---
    if (PACK_ASSETS_COMPRESS)
        list(APPEND PACK_ARGS --compress)
    endif ()

    foreach (OPTION PRELOAD RAW DATA)
        string(TOLOWER ${OPTION} OPTION_NAME)
        get_property(RULES GLOBAL PROPERTY RAWRBOX_PACK_${OPTION})

        foreach (GLOB ${RULES})
            list(APPEND PACK_ARGS --${OPTION_NAME} ${GLOB})
        endforeach ()
    endforeach ()

    set(PACK_STAMP ${CMAKE_CURRENT_BINARY_DIR}/pack_${PACK_ASSETS_TARGET}.options)
    file(WRITE ${PACK_STAMP}.tmp "${PACK_ARGS}\n${PACK_DEPENDS}")
    configure_file(${PACK_STAMP}.tmp ${PACK_STAMP} COPYONLY)
    # ----------

    set(PACK_OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PACK_ASSETS_NAME}_dir.vpk)
    add_custom_command(
            OUTPUT ${PACK_OUTPUT}
            COMMAND $<TARGET_FILE:rawrbox_vpk> pack ${PACK_OUTPUT} --cache ${CMAKE_CURRENT_BINARY_DIR}/vpk_cache_${PACK_ASSETS_TARGET} ${PACK_ARGS}
            DEPENDS rawrbox_vpk ${PACK_STAMP} ${PACK_DEPENDS}
            COMMENT "Packing ${PACK_ASSETS_TARGET}'s resources into ${PACK_OUTPUT}"
            VERBATIM)

    add_custom_target(pack_${PACK_ASSETS_TARGET} DEPENDS ${PACK_OUTPUT})
    if (TARGET ${PACK_ASSETS_TARGET})
        add_dependencies(${PACK_ASSETS_TARGET} pack_${PACK_ASSETS_TARGET})
    endif ()
endfunction()

function(mute_dependencies_warnings FILTER)
    if (WIN32)
        function(_get_all_cmake_targets out_var current_dir)
            get_property(targets DIRECTORY ${current_dir} PROPERTY BUILDSYSTEM_TARGETS)
            get_property(subdirs DIRECTORY ${current_dir} PROPERTY SUBDIRECTORIES)

            foreach (subdir ${subdirs})
                _get_all_cmake_targets(subdir_targets ${subdir})
                list(APPEND targets ${subdir_targets})
            endforeach ()

            set(${out_var} ${targets} PARENT_SCOPE)
        endfunction()

        _get_all_cmake_targets(all_targets ${CMAKE_CURRENT_LIST_DIR})
        foreach (target ${all_targets})
            get_target_property(target_type ${target} TYPE)

            if (target MATCHES ${FILTER})
                message(STATUS "Ignoring target ${target}")
            else ()
                if (
                        #target_type STREQUAL "EXECUTABLE" OR
                        target_type STREQUAL "SHARED_LIBRARY" OR
                        target_type STREQUAL "STATIC_LIBRARY")

                    message(STATUS "Disabling warnings for ${target}: ${target_type}")
                    target_compile_options(${target} PRIVATE
                            /wd4100
                            /wd4456
                            /wd4458
                            /wd4459
                            /wd4189
                            /wd4505
                            /wd4324
                            /wd4244
                            /wd4701
                            /wd4703
                            /wd4127
                            /wd4291
                            /wd4201
                            /wd4702
                            /wd4706
                            /wd4245
                            /wd5054
                            /wd4457
                            /wd4389
                            /wd4018
                            /wd4267
                    )
                endif ()
            endif ()

        endforeach ()
    endif ()
endfunction()