
#include <cube/game.h>
#include <bx/math.h>

namespace cube {
	void Game::init() {
		int width = 1024;
		int height = 768;

		this->_render = std::make_unique<rawrBOX::Renderer>();
		this->_render->setMonitor(-1);
		this->_render->setClearColor(0x443355FF);
		this->_render->setTitle("CUBE");
		this->_render->setRenderer(bgfx::RendererType::Vulkan);
		this->_render->onWindowClose += [this](auto& w) {
		};

		this->_render->initialize(this->_id, width, height, rawrBOX::RenderFlags::Features::VSYNC | rawrBOX::RenderFlags::Debug::STATS);

		// Load content
		this->_texture = std::make_shared<rawrBOX::Texture>("./content/textures/screem.png");
		this->_texture->bind(0);

		// Setup camera
		bx::mtxLookAt(this->_view, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f,  0.0f}); // Am too lazy to fix it for glm
		bx::mtxProj(this->_proj, 60.0f, static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
		bgfx::setViewTransform(this->_id, this->_view, this->_proj);
		// --------------

		this->_stencil = std::make_unique<rawrBOX::Stencil>(0, rawrBOX::Vector2(width, height));
	}

	void Game::shutdown() {
		this->_texture = nullptr;
		this->_stencil = nullptr;

		rawrBOX::Engine::shutdown();
	}

	void Game::pollEvents() {
		if(this->_render == nullptr) return;
		this->_render->pollEvents();
	}

	void Game::update(float deltaTime, int64_t gameTime) {
		if(this->_render == nullptr) return;

	}

	void Game::draw(const double alpha) {
		if(this->_render == nullptr || this->_stencil == nullptr) return;
		this->_render->swapBuffer();

			// Set View
			bgfx::setViewTransform(this->_id, this->_view, this->_proj);

			// Draw
			this->_stencil->drawTexture({10, 10}, {100, 100}, this->_texture);
			this->_stencil->draw();

		this->_render->render(); // Commit primitives
	}
}
