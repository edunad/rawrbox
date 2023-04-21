
#ifdef _WIN32
#include <windows.h>
#endif

#include <light/game.h>
using namespace rawrBox;

int main(int argc, char* argv[]) {
#ifdef _WIN32
	SetConsoleTitle("Light test");
#endif

	light::Game engine;
	engine.setFPS(60);
	engine.setTPS(66);

	engine.init();
	engine.run();

	return 0;
}
