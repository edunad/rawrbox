
#ifdef _WIN32
#include <windows.h>
#endif

#include <stencil/game.hpp>
using namespace rawrBox;

int main(int argc, char* argv[]) {
#ifdef _WIN32
	SetConsoleTitle("Stencil test");
#endif

	stencil::Game engine;
	engine.setFPS(60);
	engine.setTPS(66);

	engine.init();
	engine.run();

	return 0;
}
