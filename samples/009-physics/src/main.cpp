
#ifdef _WIN32
	#include <windows.h>
#endif

#include <physics_test/game.hpp>

int main(int  /*argc*/, char*  /*argv*/[]) {
#ifdef _WIN32
	SetConsoleTitle("PHYSICS test");
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
#endif

	physics_test::Game engine;
	engine.setFPS(60);
	engine.setTPS(66);
	engine.run();

	return 0;
}
