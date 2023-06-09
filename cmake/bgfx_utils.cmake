# FROM https://github.com/Paper-Cranes-Ltd/big2-stack/blob/main/cmake/utils.cmake
function(add_shaders_directory SHADERS_DIR TARGET_OUT_VAR)
    get_filename_component(SHADERS_DIR "${SHADERS_DIR}" ABSOLUTE)
    get_filename_component(NAMESPACE "${CMAKE_CURRENT_SOURCE_DIR}" NAME_WE)

    if(NOT EXISTS "${SHADERS_DIR}")
        message(NOTICE "Shaders directory does not exist")
        return()
    endif()

    if(CMAKE_CROSSCOMPILING AND NOT ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "${CMAKE_SYSTEM_NAME}"))
        message(STATUS "Not compiling shaders during cross-compilation")
        return()
    endif()

    set(VARYING_DEF_LOCATION "${SHADERS_DIR}/varying.def.sc") # Global
    set(SHADERS_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/include/generated/shaders/${NAMESPACE}")

    file(MAKE_DIRECTORY "${SHADERS_OUT_DIR}")
    file(GLOB_RECURSE COMPUTE_SHADER_FILES CONFIGURE_DEPENDS FOLLOW_SYMLINKS "${SHADERS_DIR}/cs_*.sc")
    file(GLOB_RECURSE VERTEX_SHADER_FILES CONFIGURE_DEPENDS FOLLOW_SYMLINKS "${SHADERS_DIR}/vs_*.sc")
    file(GLOB_RECURSE FRAGMENT_SHADER_FILES CONFIGURE_DEPENDS FOLLOW_SYMLINKS "${SHADERS_DIR}/fs_*.sc")
    file(GLOB_RECURSE INCLUDES_SHADER_FILES CONFIGURE_DEPENDS FOLLOW_SYMLINKS "${SHADERS_DIR}/*.sh")

    message("COMPUTE (${SHADERS_DIR}) ->\n: ${COMPUTE_SHADER_FILES}")
    message("VERTEX (${SHADERS_DIR}) ->\n: ${VERTEX_SHADER_FILES}")
    message("FRAGMENT (${SHADERS_DIR}) ->\n: ${FRAGMENT_SHADER_FILES}")

    bgfx_compile_shader_to_header(
        TYPE
        VERTEX
        #
        SHADERS
        ${VERTEX_SHADER_FILES}
        #
        VARYING_DEF
        "${VARYING_DEF_LOCATION}"
        #
        OUTPUT_DIR
        "${SHADERS_OUT_DIR}"
        #
        OUT_FILES_VAR
        VERTEX_OUTPUT_FILES
        #
        PROFILES
        ${PROFILES}
        #
        PARAMS
        "WERROR"
        #
        INCLUDE_DIRS
        "${SHADERS_DIR}"
        "${BGFX_DIR}/src")

    bgfx_compile_shader_to_header(
        TYPE
        FRAGMENT
        #
        SHADERS
        ${FRAGMENT_SHADER_FILES}
        #
        VARYING_DEF
        "${VARYING_DEF_LOCATION}"
        #
        OUTPUT_DIR
        "${SHADERS_OUT_DIR}"
        #
        OUT_FILES_VAR
        FRAGMENT_OUTPUT_FILES
        #
        PROFILES
        ${PROFILES}
        #
        PARAMS
        "WERROR"
        #
        INCLUDE_DIRS
        "${SHADERS_DIR}"
        "${BGFX_DIR}/src")

    bgfx_compile_shader_to_header(
        TYPE
        COMPUTE
        #
        SHADERS
        ${COMPUTE_SHADER_FILES}
        #
        OUTPUT_DIR
        "${SHADERS_OUT_DIR}"
        #
        OUT_FILES_VAR
        COMPUTE_OUTPUT_FILES
        #
        PROFILES
        ${PROFILES}
        #
        INCLUDE_DIRS
        "${SHADERS_DIR}"
        "${BGFX_DIR}/src")

    set(OUTPUT_FILES)
    list(APPEND OUTPUT_FILES ${VERTEX_OUTPUT_FILES})
    list(APPEND OUTPUT_FILES ${FRAGMENT_OUTPUT_FILES})
    list(APPEND OUTPUT_FILES ${COMPUTE_OUTPUT_FILES})

    message("OUTPUT ->\n     VERTEX ->\n    ${VERTEX_OUTPUT_FILES}")
    message("     FRAGMENT ->\n     ${FRAGMENT_OUTPUT_FILES}")
    message("     COMPUTE ->\n     ${COMPUTE_OUTPUT_FILES}")
    message("     INCLUDES ->\n     ${INCLUDES_SHADER_FILES}")

    list(LENGTH OUTPUT_FILES SHADER_COUNT)
    if(SHADER_COUNT EQUAL 0)
        return()
    endif()

    set(INCLUDE_ALL_HEADER "//===================\n// AUTO-GENERATED PLEASE DO NOT EDIT\n//====================\n\n#pragma once\n")
    foreach(OUTPUT_FILE IN LISTS OUTPUT_FILES)
        get_filename_component(OUTPUT_FILENAME "${OUTPUT_FILE}" NAME)
        string(APPEND INCLUDE_ALL_HEADER "#include <generated/shaders/${NAMESPACE}/${OUTPUT_FILENAME}>\n")
    endforeach()

    file(WRITE "${SHADERS_OUT_DIR}/all.hpp" "${INCLUDE_ALL_HEADER}")
    list(APPEND OUTPUT_FILES "${SHADERS_OUT_DIR}/all.hpp")

    string(MD5 DIR_HASH "${SHADERS_DIR}")
    set(TARGET_NAME "Shaders_${DIR_HASH}")

    add_custom_target("${DIR_HASH}" ALL DEPENDS ${OUTPUT_FILES})
    list(APPEND OUTPUT_FILES "${SHADERS_OUT_DIR}/all.hpp")

    add_library("${TARGET_NAME}" INTERFACE)
    add_dependencies("${TARGET_NAME}" shaderc "${DIR_HASH}")
    target_include_directories("${TARGET_NAME}" INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/include")

    set("${TARGET_OUT_VAR}" "${TARGET_NAME}" PARENT_SCOPE)
endfunction()
