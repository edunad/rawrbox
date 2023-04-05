# Grab source files
file(GLOB RAWRBOX_UI_SOURCES "src/*.cpp" "src/*.c" "include/*.h" "include/*.hpp" )

# Project setup
set(output_target RAWRBOX::UI)
add_library(${output_target} ${RAWRBOX_UI_SOURCES})

target_include_directories(${output_target} PUBLIC "include")
target_compile_features(${output_target} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
target_link_libraries(${output_target} PUBLIC
	${RAWRBOX_EXTRA_LIBS}
)
