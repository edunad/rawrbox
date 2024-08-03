
#ifdef _WIN32
	#include <windows.h>
#endif

#include <phys_2d_test/game.hpp>

int main(int /*argc*/, char* /*argv*/[]) {
#ifdef _WIN32
	SetConsoleTitle("PHYSICS 2D TEST");
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
#endif

	phys_2d_test::Game engine;
	engine.setFPS(120);
	engine.setTPS(66);
	engine.run();

	return 0;
}
