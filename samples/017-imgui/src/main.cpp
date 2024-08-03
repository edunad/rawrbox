

#ifdef _WIN32
	#include <windows.h>
#endif

#include <imgui/game.hpp>

int main(int /*argc*/, char* /*argv*/[]) {
#ifdef _WIN32
	SetConsoleTitle("IMGUI TEST");
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
#endif

	imgui::Game engine;
	engine.setFPS(120);
	engine.setTPS(66);
	engine.run();

	return 0;
}
