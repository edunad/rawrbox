project ("RAWRBOX.RENDER" VERSION ${RAWRBOX_VERSION} DESCRIPTION "RawrBox - Render lib" LANGUAGES CXX)
set(output_target RAWRBOX.RENDER)

# Packages ----
CPMAddPackage(
	NAME glfw
	GITHUB_REPOSITORY glfw/glfw
	GIT_TAG 3.3.8
	OPTIONS
		"GLFW_BUILD_DOCS OFF"
		"GLFW_BUILD_EXAMPLES OFF"
		"GLFW_BUILD_TESTS OFF"
		"GLFW_INSTALL OFF"
		"GLFW_USE_OSMESA OFF"
		"GLFW_VULKAN_STATIC OFF"
		"USE_MSVC_RUNTIME_LIBRARY_DLL OFF"
)

CPMAddPackage(
	NAME bgfx
	GITHUB_REPOSITORY edunad/bgfx.cmake
	GIT_TAG master
	OPTIONS
		"BGFX_BUILD_TOOLS ON"
		"BGFX_BUILD_TOOLS_SHADER ON"
		"BGFX_BUILD_EXAMPLES OFF"
		"BGFX_BUILD_TESTS OFF"

		"BGFX_OPENGLES_VERSION 30"
		"BGFX_OPENGL_VERSION 33"

		"BGFX_INSTALL_EXAMPLES OFF"

		"BGFX_AMALGAMATED ON"
		"BX_AMALGAMATED ON"
)

set_lib_runtime_mt(bimg)
set_lib_runtime_mt(bx)
set_lib_runtime_mt(bgfx)

if(bgfx_ADDED)
	message("-- Including bgfx utils")
	include("../cmake/bgfx_utils.cmake")
endif()

CPMAddPackage("gh:nemtrif/utfcpp@3.2.3")

if(RAWRBOX_ENABLE_ASSIMP_SUPPORT)
	message("-- Enabled assimp")

	CPMAddPackage(
		NAME assimp
		VERSION 5.2.5
		GITHUB_REPOSITORY assimp/assimp
		OPTIONS
			"BUILD_SHARED_LIBS OFF"
			"ASSIMP_NO_EXPORT ON"
			"ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF"
			"ASSIMP_INSTALL ON"
			"ASSIMP_BUILD_ASSIMP_TOOLS OFF"
			"ASSIMP_BUILD_TESTS OFF"
			"ASSIMP_BUILD_SAMPLES OFF"
			"ASSIMP_BUILD_DOCS OFF"
			"USE_STATIC_CRT ON"
			"ASSIMP_BUILD_ASSIMP_VIEW OFF"
			"ASSIMP_BUILD_ZLIB TRUE"
			"ASSIMP_WARNINGS_AS_ERRORS OFF"
			"ASSIMP_INSTALL_PDB OFF"
			"ASSIMP_BUILD_DRACO OFF"
			"ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF"
			"ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF"
			# IMPORT SUPPORTS
			"ASSIMP_BUILD_OBJ_IMPORTER ON"
			"ASSIMP_BUILD_FBX_IMPORTER ON"
			"ASSIMP_BUILD_DAE_IMPORTER ON"
			"ASSIMP_BUILD_STL_IMPORTER ON"
			"ASSIMP_BUILD_GLTF_IMPORTER ON"
			"ASSIMP_BUILD_3DS_IMPORTER ON"
			#"ASSIMP_BUILD_MDL_IMPORTER ON"
			"ASSIMP_BUILD_MD5_IMPORTER ON"
	)

	list(APPEND RAWRBOX_EXTRA_LIBS assimp::assimp)
endif()

# ----

if(RAWRBOX_BUILD_RESOURCES_MANAGER)
    list(APPEND RAWRBOX_EXTRA_LIBS RAWRBOX.RESOURCES)
endif()

#--------------

# Grab source files
file(GLOB_RECURSE RAWRBOX_RENDER_SOURCES "src/*.cpp" "src/*.c" "include/*.h" "include/*.hpp")

if(NOT RAWRBOX_ENABLE_ASSIMP_SUPPORT)
	foreach (TMP_PATH ${RAWRBOX_RENDER_SOURCES})
		string (FIND ${TMP_PATH} "/assimp/" EXCLUDE_DIR_FOUND)
		if (NOT ${EXCLUDE_DIR_FOUND} EQUAL -1)
			list (REMOVE_ITEM RAWRBOX_RENDER_SOURCES ${TMP_PATH})
		endif ()
	endforeach(TMP_PATH)
endif()

if(NOT RAWRBOX_BUILD_RESOURCES_MANAGER)
	foreach (TMP_PATH ${RAWRBOX_RENDER_SOURCES})
		string (FIND ${TMP_PATH} "/resources/" EXCLUDE_DIR_FOUND)
		if (NOT ${EXCLUDE_DIR_FOUND} EQUAL -1)
			list (REMOVE_ITEM RAWRBOX_RENDER_SOURCES ${TMP_PATH})
		endif ()
	endforeach(TMP_PATH)
endif()

# -----------------

# Compile shaders
if(LINUX)
	set(PROFILES 120 300_es spirv)
else()
	set(PROFILES 120 300_es spirv s_4_0 s_5_0)
endif()

add_shaders_directory(./shaders RAWRBOX_SHADERS_TARGET_NAME)
message("SHADERS: ${RAWRBOX_SHADERS_TARGET_NAME}")
# ------------------

# Copy content - RENDER
add_custom_target(copy_resources_render ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_SOURCE_DIR}/${RAWRBOX_CONTENT_FOLDER}
        ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/${RAWRBOX_CONTENT_FOLDER}
        COMMENT "Copying RENDERER resources into binary directory")
# ------------------

# Project setup
add_library(${output_target} ${RAWRBOX_LIBRARY_TYPE} ${RAWRBOX_RENDER_SOURCES})
add_dependencies(${output_target} copy_resources_render)
target_compile_definitions(${output_target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX INTERFACE RAWRBOX_RENDER)
target_include_directories(${output_target} PUBLIC "include" )
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC ${RAWRBOX_SHADERS_TARGET_NAME}
	RAWRBOX.MATH
	RAWRBOX.ENGINE
	${RAWRBOX_EXTRA_LIBS}

	fmt
	magic_enum
	utf8cpp

	bgfx
	bx
	glfw
)

set_lib_runtime_mt(${output_target})

# TEST ----
include(../cmake/catch2.cmake)
#--------------
