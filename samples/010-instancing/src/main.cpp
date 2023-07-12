
#ifdef _WIN32
	#include <windows.h>
#endif

#include <instance_test/game.hpp>

int main(int argc, char* argv[]) {
#ifdef _WIN32
	SetConsoleTitle("INSTANCE TEST");
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
#endif

	instance_test::Game engine;
	engine.setFPS(60);
	engine.setTPS(66);
	engine.run();

	return 0;
}
