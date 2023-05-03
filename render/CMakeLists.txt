
# INSTALL GLFW & BGFX -----
FetchContent_GetProperties(glfw3)
if(NOT glfw3_POPULATED)
	FetchContent_Declare(glfw3
		GIT_REPOSITORY "https://github.com/glfw/glfw"
		GIT_TAG "origin/master"
		GIT_PROGRESS TRUE
	)

	FetchContent_MakeAvailable(glfw3)
endif()

FetchContent_GetProperties(bgfx)
if(NOT bgfx_POPULATED)
	FetchContent_Declare(bgfx
		GIT_REPOSITORY "https://github.com/edunad/bgfx.cmake.git"
		GIT_TAG "origin/master"
		GIT_PROGRESS TRUE
	)

	FetchContent_MakeAvailable(bgfx)
endif()
# ---------------------

# Packages ----
find_package(Freetype REQUIRED MODULE)
find_package(utf8cpp REQUIRED)

if(RAWRBOX_ENABLE_ASSIMP_SUPPORT)
	message("-- Enabled assimp")
	find_package(assimp REQUIRED)

	list(APPEND RAWRBOX_EXTRA_LIBS assimp::assimp)
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
# -----------------

# Compile shaders
if(LINUX)
	set(PROFILES 300_es spirv)
else()
	set(PROFILES 300_es spirv s_4_0 s_5_0)
endif()

add_shaders_directory(./shaders RAWRBOX_SHADERS_TARGET_NAME)
message("SHADERS: ${RAWRBOX_SHADERS_TARGET_NAME}")
# ------------------

# Project setup
set(output_target RAWRBOX.RENDER)
add_library(${output_target} ${RAWRBOX_RENDER_SOURCES})
target_compile_definitions(${output_target} PUBLIC _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_include_directories(${output_target} PUBLIC "include" ${FREETYPE_INCLUDE_DIRS})
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC ${RAWRBOX_SHADERS_TARGET_NAME}
	RAWRBOX.MATH
	RAWRBOX.UTILS

	fmt::fmt
	Freetype::Freetype
	utf8cpp

	bgfx
	bx
	glfw

	${RAWRBOX_EXTRA_LIBS}
)

# Copy content
add_custom_command(
	TARGET ${output_target} PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_HOME_DIRECTORY}/render/${RAWRBOX_CONTENT_FOLDER} ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/${RAWRBOX_CONTENT_FOLDER}
)
# ------------------
