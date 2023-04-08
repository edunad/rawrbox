
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
# ---------------------------------

# Packages ----
find_package(stb REQUIRED)
#--------------

add_shaders_directory(${RAWRBOX_CONTENT_FOLDER}/shaders RAWRBOX_SHADERS_TARGET_NAME)
message("SHADERS: ${RAWRBOX_SHADERS_TARGET_NAME}")

# Grab source files
file(GLOB_RECURSE RAWRBOX_RENDER_SOURCES "src/*.cpp" "src/*.c" "include/*.h" "include/*.hpp" )

# Project setup
set(output_target RAWRBOX.RENDER)
add_library(${output_target} ${RAWRBOX_RENDER_SOURCES})

target_include_directories(${output_target} PUBLIC "include" )
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PRIVATE "${RAWRBOX_SHADERS_TARGET_NAME}" PUBLIC
	RAWRBOX.MATH
	RAWRBOX.UTILS

	stb::stb
	fmt::fmt
	glfw

	bgfx
	bx

	${RAWRBOX_EXTRA_LIBS}
)
