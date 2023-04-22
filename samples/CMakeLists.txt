

add_custom_target(copy_resources ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_SOURCE_DIR}/content
        ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/content
        COMMENT "Copying resources into binary directory")

add_subdirectory ("lights")
add_subdirectory ("stencil")
add_subdirectory ("assimp")
add_subdirectory ("post_process")
