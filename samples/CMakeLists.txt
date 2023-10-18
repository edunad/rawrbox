# Copy content - SAMPLES
add_custom_target(
    copy_resources_samples ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets
                                       ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${RAWRBOX_CONTENT_FOLDER}
    COMMENT "Copying SAMPLES assets into binary directory")
# ------------------

add_subdirectory("001-stencil")
add_subdirectory("002-generated-models")
#
#if(RAWRBOX_BUILD_RAWRBOX_ASSIMP)
#    add_subdirectory("003-assimp")
#endif()
#
#add_subdirectory("004-light-support")
#add_subdirectory("005-post-process")
#add_subdirectory("015-gpu-picking")
#
#if(RAWRBOX_BUILD_RAWRBOX_BASS)
#    add_subdirectory("006-bass-loading")
#endif()
#
#add_subdirectory("007-particle-system")
#
#if(RAWRBOX_BUILD_RAWRBOX_UI)
#    add_subdirectory("008-ui")
#endif()
#
#if(RAWRBOX_BUILD_RAWRBOX_3D_PHYSICS)
#    add_subdirectory("009-physics-3d")
#endif()
#
#add_subdirectory("010-instancing")
#add_subdirectory("011-decals")
#
#if(RAWRBOX_BUILD_RAWRBOX_WEBM)
#    add_subdirectory("012-webm")
#endif()
#
#if(RAWRBOX_BUILD_RAWRBOX_2D_PHYSICS)
#    add_subdirectory("013-physics-2d")
#endif()
#
#if(RAWRBOX_BUILD_RAWRBOX_SCRIPTING)
#    add_subdirectory("014-scripting")
#endif()
