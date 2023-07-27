
#ifdef _WIN32
	#include <windows.h>
#endif

#include <webm_test/game.hpp>

int main(int argc, char* argv[]) {
#ifdef _WIN32
	SetConsoleTitle("WEBM TEST");
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
#endif

	webm_test::Game engine;
	engine.setFPS(60);
	engine.setTPS(66);
	engine.run();

	return 0;
}
