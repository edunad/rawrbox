
#include <rawrbox/render/static.hpp>

#include <stencil/game.hpp>

#include <bx/bx.h>
#include <bx/math.h>

#include <vector>

namespace stencil {
	void Game::init() {
		int width = 1024;
		int height = 768;

		this->_window = std::make_unique<rawrBox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("STENCIL TEST");
		this->_window->setRenderer(bgfx::RendererType::Count);
		this->_window->onResize += [this](auto& w, auto& size) {
			if (this->_render == nullptr) return;
			this->_render->resizeView(size);
		};

		this->_window->onWindowClose += [this](auto& w) {
			this->shutdown();
		};

		this->_window->initialize(width, height, rawrBox::WindowFlags::Debug::TEXT | rawrBox::WindowFlags::Window::WINDOWED);

		this->_render = std::make_shared<rawrBox::Renderer>(0, this->_window->getSize());
		this->_render->setClearColor(0x443355FF);

		this->_textEngine = std::make_unique<rawrBox::TextEngine>();

		// Load content ---
		this->loadContent();
		// -----
	}

	void Game::loadContent() {
		this->_render->upload();

		// Fonts ----
		this->_font = &this->_textEngine->load("./content/fonts/droidsans.ttf", 28);
		this->_font2 = &this->_textEngine->load("./content/fonts/visitor1.ttf", 18);
		this->_font3 = &this->_textEngine->load("cour.ttf", 12);

		// Textures ---
		this->_texture = std::make_shared<rawrBox::TextureImage>("./content/textures/screem.png");
		this->_texture->upload();

		this->_texture2 = std::make_shared<rawrBox::TextureGIF>("./content/textures/meow3.gif");
		this->_texture2->upload();
	}

	void Game::shutdown() {
		this->_render = nullptr;
		this->_textEngine = nullptr;
		this->_texture = nullptr;
		this->_texture2 = nullptr;

		rawrBox::Engine::shutdown();
	}

	void Game::pollEvents() {
		if (this->_window == nullptr) return;
		this->_window->pollEvents();
	}

	void Game::update(float deltaTime, int64_t gameTime) {
		if (this->_render == nullptr) return;
	}

	void Game::drawOverlay() {
		auto& stencil = this->_render->getStencil();

		stencil.begin();

		stencil.pushOffset({20, 50});
		stencil.pushRotation({counter * 50.5F, {50, 50}});
		stencil.drawBox({0, 0}, {100, 100}, rawrBox::Colors::Green);
		stencil.popRotation();

		stencil.pushOffset({100, 0});
		stencil.pushScale({1.F, -1.F});
		stencil.drawBox({0, 0}, {100, 100}, rawrBox::Colors::Red);
		stencil.popScale();
		stencil.popOffset();

		stencil.pushOffset({200, 0});
		stencil.pushOutline({1.F, 2.F});
		stencil.drawBox({0, 0}, {100, 100}, rawrBox::Colors::Purple);
		stencil.popOutline();

		stencil.pushOutline({2.F});
		stencil.drawBox({25, 25}, {50, 50}, rawrBox::Colors::Purple);
		stencil.popOutline();
		stencil.popOffset();

		stencil.pushOffset({300, 0});
		stencil.drawTriangle({0, 0}, {0, 0}, rawrBox::Colors::Blue, {0, 100}, {0, 1}, rawrBox::Colors::Blue, {100, 0}, {0, 1}, rawrBox::Colors::Blue);
		stencil.popOffset();

		stencil.pushOffset({400, 0});
		stencil.pushOutline({2.F});
		stencil.drawTriangle({15, 15}, {0, 0}, rawrBox::Colors::Blue, {15, 65}, {0, 1}, rawrBox::Colors::Blue, {65, 15}, {0, 1}, rawrBox::Colors::Blue);
		stencil.popOutline();

		stencil.pushOutline({1.F, 1.F});
		stencil.drawTriangle({0, 0}, {0, 0}, rawrBox::Colors::Blue, {0, 100}, {0, 1}, rawrBox::Colors::Blue, {100, 0}, {0, 1}, rawrBox::Colors::Blue);
		stencil.popOutline();
		stencil.popOffset();

		stencil.pushOffset({500, 0});
		stencil.drawCircle({0, 0}, {100, 100}, rawrBox::Colors::Orange, 16, 0, std::fmod(counter * 50.5F, 360.F));
		stencil.popOffset();

		stencil.pushOffset({600, 0});
		stencil.pushOutline({1.F, 0.25F});
		stencil.drawCircle({0, 0}, {100, 100}, rawrBox::Colors::Red, 16, 0.F, std::fmod(counter * 50.5F, 360.F));
		stencil.popOutline();

		stencil.pushOutline({2.F});
		stencil.drawCircle({25, 25}, {50, 50}, rawrBox::Colors::Red, 16, 0.F, std::fmod(counter * 50.5F, 360.F));
		stencil.popOutline();
		stencil.popOffset();

		stencil.pushOffset({700, 0});
		stencil.drawLine({0, 0}, {100, 100}, rawrBox::Colors::Red);

		stencil.pushOutline({1.F, 2.F});
		stencil.drawLine({100, 0}, {0, 100}, rawrBox::Colors::Blue);
		stencil.popOutline();

		stencil.pushOutline({3.F, 2.F});
		stencil.drawLine({50, 0}, {50, 100}, rawrBox::Colors::Purple);
		stencil.popOutline();
		stencil.popOffset();

		stencil.pushOffset({800, 0});
		stencil.drawTexture({0, 0}, {100, 100}, this->_texture);
		stencil.popOffset();

		stencil.pushOffset({900, 0});
		stencil.drawTexture({0, 0}, {100, 100}, this->_texture2);
		stencil.popOffset();
		stencil.popOffset();

		stencil.pushOffset({20, 200});
		stencil.drawText(this->_font, "Cat ipsum dolor sit amet, steal raw zucchini off kitchen counter. $£%&", {});

		auto size = this->_font2->getStringSize("Cat!!");
		stencil.pushRotation({counter * 50.5F, (size / 2.F) + rawrBox::Vector2f(0, 40)});
		stencil.drawText(this->_font2, "Cat!!", {0, 40});
		stencil.popRotation();

		stencil.drawText(this->_font3, "MeW MeW MeW!", {0, 75});
		stencil.popOffset();

		stencil.end();
	}

	void Game::draw(const double alpha) {
		if (this->_render == nullptr) return;
		this->_render->swapBuffer(); // Clean up and set renderer

		bgfx::setViewTransform(rawrBox::CURRENT_VIEW_ID, nullptr, nullptr);
		bgfx::dbgTextPrintf(1, 1, 0x0f, "STENCIL TESTS ----------------------------------------------------------------------------------------------------------------");

		this->drawOverlay();

		this->_render->render(); // Commit primitives
		this->_texture2->step();

		counter += 0.1F;
	}
} // namespace stencil
