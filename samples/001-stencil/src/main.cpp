
#ifdef _WIN32
	#include <windows.h>
#endif

#include <stencil/game.hpp>

int main(int  /*argc*/, char*  /*argv*/[]) {
#ifdef _WIN32
	SetConsoleTitle("Stencil test");
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
#endif

	stencil::Game engine;
	engine.setFPS(60);
	engine.setTPS(66);
	engine.run();

	return 0;
}
