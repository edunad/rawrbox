
#ifdef _WIN32
	#include <windows.h>
#endif

#include <anims/game.hpp>
using namespace rawrbox;

int main(int argc, char* argv[]) {
#ifdef _WIN32
	SetConsoleTitle("Assim-animations test");
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
#endif

	anims::Game engine;
	engine.setFPS(60);
	engine.setTPS(66);

	engine.init();
	engine.run();

	return 0;
}
