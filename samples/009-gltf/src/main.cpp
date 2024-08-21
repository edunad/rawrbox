
#ifdef _WIN32
	#include <windows.h>
#endif

#include <gltf/game.hpp>

int main(int /*argc*/, char* /*argv*/[]) {
#ifdef _WIN32
	SetConsoleTitle("GLTF test");
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
#endif

	gltf::Game engine;
	engine.setFPS(120);
	engine.setTPS(66);
	engine.run();

	return 0;
}
