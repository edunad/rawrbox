
#ifdef _WIN32
	#include <windows.h>
#endif

#include <cube/game.h>
using namespace rawrBox;

int main(int argc, char* argv[]) {
	#ifdef _WIN32
		SetConsoleTitle("YOU CANNOT CONTAIN THE POWER OF THE CUBE");
	#endif

	cube::Game engine;
	engine.setFPS(60);
	engine.setTPS(10);
	engine.init();
	engine.run();

	return 0;
}
