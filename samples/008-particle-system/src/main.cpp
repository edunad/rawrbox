
#ifdef _WIN32
	#include <windows.h>
#endif

#include <particle_test/game.hpp>

int main(int argc, char* argv[]) {
#ifdef _WIN32
	SetConsoleTitle("PARTICLE test");
#endif

	particle_test::Game engine;
	engine.setFPS(60);
	engine.setTPS(66);

	engine.init();
	engine.run();

	return 0;
}
