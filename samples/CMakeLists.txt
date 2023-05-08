

add_custom_target(copy_resources ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_SOURCE_DIR}/content
        ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/content
        COMMENT "Copying resources into binary directory")


add_subdirectory ("001-stencil")
add_subdirectory ("002-generated-models")
add_subdirectory ("003-assimp-loading")
add_subdirectory ("004-light-support")
add_subdirectory ("005-post-process")
add_subdirectory ("006-assimp-animations")
add_subdirectory ("007-bass-loading")
add_subdirectory ("008-particle-system")
