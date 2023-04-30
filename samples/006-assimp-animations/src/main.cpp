
#ifdef _WIN32
#include <windows.h>
#endif

#include <anims/game.hpp>
using namespace rawrBox;

int main(int argc, char* argv[]) {
#ifdef _WIN32
	SetConsoleTitle("Post-process test");
#endif

	anims::Game engine;
	engine.setFPS(60);
	engine.setTPS(66);

	engine.init();
	engine.run();

	return 0;
}
