
#ifdef _WIN32
	#include <windows.h>
#endif

#include <model/game.hpp>

int main(int argc, char* argv[]) {
#ifdef _WIN32
	SetConsoleTitle("Post-process test");
#endif

	model::Game engine;
	engine.setFPS(60);
	engine.setTPS(66);

	engine.init();
	engine.run();

	return 0;
}
