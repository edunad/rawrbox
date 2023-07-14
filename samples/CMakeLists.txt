# Copy content - SAMPLES
add_custom_target(
    copy_resources_samples ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/${RAWRBOX_CONTENT_FOLDER}
                                       ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/${RAWRBOX_CONTENT_FOLDER}
    COMMENT "Copying SAMPLES resources into binary directory")
# ------------------

add_subdirectory("001-stencil")
add_subdirectory("002-generated-models")

if(RAWRBOX_ENABLE_ASSIMP_SUPPORT)
    add_subdirectory("003-assimp-loading")
    add_subdirectory("004-light-support")
    add_subdirectory("005-post-process")
endif()

if(RAWRBOX_ENABLE_BASS_SUPPORT)
    add_subdirectory("006-bass-loading")
endif()

add_subdirectory("007-particle-system")

if(RAWRBOX_BUILD_UI)
    add_subdirectory("008-ui")
endif()

if(RAWRBOX_BUILD_PHYSICS)
    add_subdirectory("009-physics")
endif()

add_subdirectory("010-instancing")
add_subdirectory("011-decals")
