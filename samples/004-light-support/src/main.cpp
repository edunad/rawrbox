
#ifdef _WIN32
	#include <windows.h>
#endif

#include <light/game.hpp>
using namespace rawrbox;

int main(int argc, char* argv[]) {
#ifdef _WIN32
	SetConsoleTitle("Light test");
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
#endif

	light::Game engine;
	engine.setFPS(60);
	engine.setTPS(66);
	engine.run();

	return 0;
}
