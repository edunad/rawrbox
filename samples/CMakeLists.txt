# Pack content - SAMPLES
add_resources(
        FOLDER assets
        RAW # ATLAS + PIXEL READING
        "textures/instance_test.png"
        "textures/decals.png"
        "textures/particles.png"
        "textures/crate_hl1.png"
        "textures/screem.png"
)

pack_assets(TARGET samples)
# ------------------

add_subdirectory("001-stencil")
add_subdirectory("002-generated-models")
if (RAWRBOX_BUILD_RAWRBOX_GLTF)
    add_subdirectory("003-light")
endif ()
add_subdirectory("004-instancing")
add_subdirectory("005-post-process")
add_subdirectory("006-decals")
add_subdirectory("007-particle-system")

if (RAWRBOX_BUILD_RAWRBOX_UI)
    add_subdirectory("008-ui")
endif ()

if (RAWRBOX_BUILD_RAWRBOX_GLTF)
    add_subdirectory("009-gltf")
endif ()

if (RAWRBOX_BUILD_RAWRBOX_BASS)
    add_subdirectory("010-bass-audio")
endif ()

if (RAWRBOX_BUILD_RAWRBOX_3D_PHYSICS)
    add_subdirectory("011-physics-3D")
endif ()

if (RAWRBOX_BUILD_RAWRBOX_2D_PHYSICS)
    add_subdirectory("012-physics-2D")
endif ()

if (RAWRBOX_BUILD_RAWRBOX_WEBM)
    add_subdirectory("013-webm")
endif ()

if (RAWRBOX_BUILD_RAWRBOX_SCRIPTING)
    add_subdirectory("014-scripting")
endif ()

add_subdirectory("015-gpu-picking")

if (RAWRBOX_BUILD_RAWRBOX_STEAMWORKS)
    add_subdirectory("016-steamworks")
endif ()

if (RAWRBOX_BUILD_RAWRBOX_IMGUI)
    add_subdirectory("017-imgui")
endif ()

add_subdirectory("018-camera")