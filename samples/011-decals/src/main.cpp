
#ifdef _WIN32
	#include <windows.h>
#endif

#include <decal_test/game.hpp>

int main(int  /*argc*/, char*  /*argv*/[]) {
#ifdef _WIN32
	SetConsoleTitle("DECALS TEST");
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
#endif

	decal_test::Game engine;
	engine.setFPS(60);
	engine.setTPS(66);
	engine.run();

	return 0;
}
