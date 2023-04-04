include(GNUInstallDirs)

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
		GIT_REPOSITORY "https://github.com/bkaradzic/bgfx.cmake.git"
		GIT_TAG "v1.118.8417-420"
		PATCH_COMMAND git reset --hard && git apply --ignore-space-change --ignore-whitespace "${CMAKE_CURRENT_SOURCE_DIR}/patches/bgfx-cmake.patch" && git submodule foreach git pull origin master
		GIT_PROGRESS TRUE
	)

	FetchContent_MakeAvailable(bgfx)
endif()
# ---------------------

add_shaders_directory(${RAWRBOX_CONTENT_FOLDER}/shaders SHADERS_TARGET_NAME)
message("SHADERS: ${SHADERS_TARGET_NAME}")


# Grab source files
file(GLOB RAWRBOX_RENDER_SOURCES "src/*.cpp" "src/*.c" "include/*.h" "include/*.hpp" )

# Project setup
set(output_target RAWRBOX.RENDER)
add_library(${output_target} ${RAWRBOX_RENDER_SOURCES})

target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC
	RAWRBOX.MATH
	RAWRBOX.UTILS

	fmt::fmt
	glfw

	bgfx
	bx

	${RAWRBOX_EXTRA_LIBS}
)
