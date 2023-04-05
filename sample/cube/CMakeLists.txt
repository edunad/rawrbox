project("CUBE_SAVIOR_OF_THE_POLYVERSE")

# Grab source files
file(GLOB RAWRBOX_SAMPLE_SOURCES "include/*.h" "include/*.hpp" "src/*.cpp" "src/*.c" )

# Project setup
set(output_target CUBE_SAVIOR_OF_THE_POLYVERSE)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE})

add_executable(${output_target} ${RAWRBOX_SAMPLE_SOURCES} "ias.rc")
target_include_directories(${output_target} PRIVATE "include")
target_compile_features(${output_target} PRIVATE cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PRIVATE RAWRBOX.RENDER)
target_compile_definitions(${output_target} PUBLIC
	_CRT_SECURE_NO_WARNINGS
	NOMINMAX
)

if(NOT WIN32)
	set_target_properties(${output_target} PROPERTIES LINK_FLAGS -Wl,-rpath,'\$ORIGIN')
else()
	add_compile_options("/EHsc")
endif()


# symlink content
if (NOT EXISTS ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/content)
	add_custom_command(
		TARGET ${output_target} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/content ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE}/content
	)
endif()
