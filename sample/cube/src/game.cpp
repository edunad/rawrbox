
#include <cube/game.h>
#include <bx/math.h>

namespace cube {
	void Game::init() {
		int width = 1024;
		int height = 768;

		this->_window = std::make_unique<rawrBox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("CUBE");
		this->_window->setRenderer(bgfx::RendererType::Vulkan);
		this->_window->onResize += [this](auto& w, auto& size) {
			if(this->_render == nullptr) return;
			this->_render->resizeView({}, size);
		};
		this->_window->onWindowClose += [this](auto& w) {
			this->shutdown();
		};

		this->_window->initialize(width, height, rawrBox::WindowFlags::Features::VSYNC | rawrBox::WindowFlags::Features::RESIZABLE |  rawrBox::WindowFlags::Debug::TEXT | rawrBox::WindowFlags::Window::WINDOWED);

		this->_render = std::make_unique<rawrBox::Renderer>(0, rawrBox::Vector2i(width, height));
		this->_render->setClearColor(0x443355FF);
		this->_render->initialize();

		// Load content
		this->_texture = std::make_shared<rawrBox::Texture>("./content/textures/screem.png");
		this->_texture->upload();

		this->_texture2 = std::make_shared<rawrBox::Texture>("./content/textures/cat_nya.png");
		this->_texture2->upload();

		this->_texture3 = std::make_shared<rawrBox::AnimatedTexture>("./content/textures/meow3.gif");
		this->_texture3->upload();

		this->_texture4 = std::make_shared<rawrBox::AnimatedTexture>("./content/textures/no_loop.gif");
		this->_texture4->setLoop(false);
		this->_texture4->upload();
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
		auto stencil = this->_render->getStencil();

		this->_render->swapBuffer(); // Clean up and set renderer

			// 2D Drawing
			bgfx::setViewTransform(this->_render->getID(), NULL, this->_proj);

			//bgfx::setViewTransform(this->_render->getID(), NULL, this->_proj);
			//stencil->drawBox({0, 0}, {100, 100}, rawrBox::Color::debug(static_cast<int>(counter)));
			stencil->drawTexture({100, 100}, {100, 100}, this->_texture);
			stencil->drawTexture({400, 100}, {100, 100}, this->_texture2);

			stencil->drawTexture({400, 300}, {100, 100}, this->_texture3->getHandle());
			this->_texture3->step();

			stencil->drawTexture({400, 600}, {200, 100}, this->_texture4->getHandle());
			this->_texture4->step();

			stencil->draw();
			// ----------------

			// 3D Drawing
			bgfx::setViewTransform(this->_render->getID(), this->_view, this->_proj);
			// ----------------

			bgfx::dbgTextPrintf(0, 1, 0x0f, "Color can be changed with ANSI \x1b[9;me\x1b[10;ms\x1b[11;mc\x1b[12;ma\x1b[13;mp\x1b[14;me\x1b[0m code too.");
		this->_render->render(); // Commit primitives
		counter++;
	}
}
