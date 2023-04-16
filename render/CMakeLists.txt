
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

# libraries 3th party
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "")
set(GLFW_INSTALL OFF CACHE INTERNAL "")

set(BGFX_BUILD_TOOLS ON CACHE INTERNAL "")
set(BGFX_BUILD_TOOLS_SHADER ON CACHE INTERNAL "")
set(BGFX_BUILD_TOOLS_GEOMETRY OFF CACHE INTERNAL "")
set(BGFX_BUILD_TOOLS_TEXTURE OFF CACHE INTERNAL "")
set(BGFX_BUILD_TOOLS_BIN2C OFF CACHE INTERNAL "")

set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_SAMPLES OFF)
set(ASSIMP_BUILD_DOCS OFF)
set(ASSIMP_BUILD_ASSIMP_VIEW OFF)
# ---------------------------------

# Packages ----
find_package(Freetype REQUIRED MODULE)
find_package(utf8cpp REQUIRED)

if(RAWRBOX_BUILD_RENDER_ASSIMP_SUPPORT)
	message("-- Enabled assimp")
	find_package(assimp REQUIRED)

	list(APPEND CMAKE_CXX_FLAGS "-DASSIMP_USE_STB_IMAGE_STATIC")
	list(APPEND RAWRBOX_EXTRA_LIBS assimp::assimp)
endif()
#--------------

# Grab source files
file(GLOB_RECURSE RAWRBOX_RENDER_SOURCES "src/*.cpp" "src/*.c" "include/*.h" "include/*.hpp")
if(NOT RAWRBOX_BUILD_RENDER_ASSIMP_SUPPORT)
	foreach (TMP_PATH ${RAWRBOX_RENDER_SOURCES})
		string (FIND ${TMP_PATH} "/assimp/" EXCLUDE_DIR_FOUND)
		if (NOT ${EXCLUDE_DIR_FOUND} EQUAL -1)
			list (REMOVE_ITEM RAWRBOX_RENDER_SOURCES ${TMP_PATH})
		endif ()
	endforeach(TMP_PATH)
endif()
# -----------------

# Compile shaders
add_shaders_directory(${RAWRBOX_CONTENT_FOLDER}/shaders RAWRBOX_SHADERS_TARGET_NAME)
message("SHADERS: ${RAWRBOX_SHADERS_TARGET_NAME}")
# ------------------

# Project setup
set(output_target RAWRBOX.RENDER)
add_library(${output_target} ${RAWRBOX_RENDER_SOURCES})
target_compile_definitions(${output_target} PUBLIC _CRT_SECURE_NO_WARNINGS NOMINMAX)
target_include_directories(${output_target} PUBLIC "include" ${FREETYPE_INCLUDE_DIRS})
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PRIVATE "${RAWRBOX_SHADERS_TARGET_NAME}" PUBLIC
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
