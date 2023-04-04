project("CUBE_SAVIOR_OF_THE_POLYVERSE")

# Grab source files
file(GLOB RAWRBOX_SAMPLE_SOURCES "include/*.h" "include/*.hpp" "src/*.cpp" "src/*.c" )
list(REMOVE_ITEM RAWRBOX_SAMPLE_SOURCES "src/main.cpp")

# Project setup
set(output_target CUBE_SAVIOR_OF_THE_POLYVERSE)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/${RAWRBOX_OUTPUT_BIN}/${CMAKE_BUILD_TYPE})

add_executable(${output_target} "src/main.cpp" "ias.rc")
target_include_directories(${output_target} PRIVATE "include")
target_compile_features(${output_target} PRIVATE cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PRIVATE RAWRBOX.RENDER)
target_compile_definitions(${output_target} PUBLIC
	SOL_ALL_SAFETIES_ON=1
	_CRT_SECURE_NO_WARNINGS
	GLM_FORCE_LEFT_HANDED
	NOMINMAX
)

if(NOT WIN32)
	set_target_properties(${output_target} PROPERTIES LINK_FLAGS -Wl,-rpath,'\$ORIGIN')
else()
	add_compile_options("/EHsc")
endif()
