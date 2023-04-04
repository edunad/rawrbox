
#ifdef _WIN32
	#include <windows.h>
#endif

#include <rawrbox/render/renderer.h>

#include <bgfx/platform.h>
#include <string>

using namespace rawrbox::render;

int main(int argc, char* argv[]) {
	int width = 1024;
	int height = 768;

	#ifdef _WIN32
		SetConsoleTitle("YOU CANNOT CONTAIN THE POWER OF THE CUBE");
	#endif

	Renderer render;
	render.setMonitor(-1);
	render.setClearColor(0x443355FF);
	render.setTitle("CUBE");
	render.setRenderer(bgfx::RendererType::Vulkan);

	render.initialize(width, height, RenderFlags::Features::VSYNC | RenderFlags::Debug::STATS);
}
