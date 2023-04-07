
#include <cube/game.h>
#include <bx/math.h>

namespace cube {
	void Game::init() {
		int width = 1024;
		int height = 768;

		this->_window = std::make_unique<rawrBox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("CUBE");
		this->_window->setRenderer(bgfx::RendererType::Count);
		this->_window->onResize += [this](auto& w, auto& size) {
			if(this->_render == nullptr) return;
			this->_render->resizeView(size);
		};
		this->_window->onWindowClose += [this](auto& w) {
			this->shutdown();
		};

		this->_window->initialize(width, height, rawrBox::WindowFlags::Features::VSYNC | rawrBox::WindowFlags::Features::RESIZABLE | rawrBox::WindowFlags::Features::MULTI_THREADED | rawrBox::WindowFlags::Debug::TEXT | rawrBox::WindowFlags::Debug::STATS | rawrBox::WindowFlags::Window::WINDOWED);

		this->_render = std::make_shared<rawrBox::Renderer>(0, rawrBox::Vector2i(width, height));
		this->_render->setClearColor(0x443355FF);
		this->_render->initialize();

		// Load content
		this->_texture = std::make_shared<rawrBox::TextureImage>("./content/textures/screem.png");
		this->_texture->upload();

		this->_texture2 = std::make_shared<rawrBox::TextureImage>("./content/textures/cat_nya.png");
		this->_texture2->upload();

		this->_texture3 = std::make_shared<rawrBox::TextureGIF>("./content/textures/meow3.gif");
		this->_texture3->setSpeed(0.5f);
		this->_texture3->upload();
		// -----

		// Setup camera
		bx::mtxProj(this->_proj, 60.0f, static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth); // Crashes on linux
		bx::mtxLookAt(this->_view, {0.0f, 0.0f, -5.0f}, {0.0f, 0.0f, 0.0f});
		// --------------

	}

	void Game::shutdown() {
		this->_texture = nullptr;
		this->_render = nullptr;

		rawrBox::Engine::shutdown();
	}

	void Game::pollEvents() {
		if(this->_window == nullptr) return;
		this->_window->pollEvents();
	}

	void Game::update(float deltaTime, int64_t gameTime) {
		if(this->_render == nullptr) return;

	}

	float counter = 0;
	void Game::draw(const double alpha) {
		if(this->_render == nullptr) return;

		this->_render->swapBuffer(); // Clean up and set renderer
			bgfx::setViewTransform(this->_render->getID(), NULL, this->_proj);
			bgfx::dbgTextPrintf(0, 4, 0x0f, "CLEAR ME SENPAI");

			auto& stencil = this->_render->getStencil();

			stencil.begin();
				stencil.drawBox({400 +std::cos(counter * 0.5f) * 200.f, 400 + std::sin(counter * 0.5f) * 200.f}, {100, 100}, rawrBox::Color::debug(static_cast<int>(counter)));
				stencil.drawTexture({100 + std::cos(counter * 1.5f) * 5.f, 100 + std::sin(counter * 1.5f) * 5.f}, {100, 100}, this->_texture);
				stencil.drawTexture({400, 100}, {100, 100}, this->_texture2);
				stencil.drawTexture({400, 300}, {100, 100}, this->_texture3);
				this->_texture3->step();
			stencil.end();

			bgfx::dbgTextPrintf(0, 1, 0x0f, "Color can be changed with ANSI \x1b[9;me\x1b[10;ms\x1b[11;mc\x1b[12;ma\x1b[13;mp\x1b[14;me\x1b[0m code too.");
			bgfx::setViewTransform(this->_render->getID(), this->_view, this->_proj);
		this->_render->render(); // Commit primitives

		counter++;
	}
}
