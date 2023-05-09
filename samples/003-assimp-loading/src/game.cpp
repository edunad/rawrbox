#include <rawrbox/render/model/light/manager.hpp>
#include <rawrbox/utils/keys.hpp>

#include <assimp/game.hpp>

#include <bx/bx.h>
#include <bx/math.h>
#include <bx/timer.h>

#include <vector>

namespace assimp {
	void Game::init() {
		int width = 1024;
		int height = 768;

		this->_window = std::make_unique<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("ASSIMP TEST");
		this->_window->setRenderer(bgfx::RendererType::Count);
		this->_window->onResize += [this](auto& w, auto& size) {
			if (this->_render == nullptr) return;
			this->_render->resizeView(size);
		};

		this->_window->onWindowClose += [this](auto& w) {
			this->shutdown();
		};

		this->_window->initialize(width, height, rawrbox::WindowFlags::Debug::TEXT | rawrbox::WindowFlags::Window::WINDOWED);

		this->_render = std::make_shared<rawrbox::Renderer>(0, this->_window->getSize());
		this->_render->setClearColor(0x000000FF);

		// Setup camera
		this->_camera = std::make_shared<rawrbox::CameraOrbital>(this->_window.get());
		this->_camera->setPos({0.F, 5.F, -5.F});
		this->_camera->setAngle({0.F, bx::toRad(-45), 0.F, 0.F});
		// --------------

		this->_textEngine = std::make_unique<rawrbox::TextEngine>();

		// Load content ---
		this->loadContent();
		// -----
	}

	void Game::loadContent() {
		this->_render->upload();

		// Fonts -----
		this->_font = &this->_textEngine->load("cour.ttf", 16);
		// ------

		// Assimp test ---
		this->_model->setPos({10, 0, 0});
		this->_model->load("./content/models/ps1_phasmophobia/Phasmaphobia_Semi.fbx");
		this->_model->upload();

		this->_model2->setPos({0, 0, 0});
		this->_model2->load("./content/models/ps1_phasmophobia/Phasmaphobia_Semi.fbx", rawrbox::ModelLoadFlags::IMPORT_TEXTURES);
		this->_model2->upload();

		this->_model3->setPos({-10, 0, 0});
		this->_model3->load("./content/models/ps1_phasmophobia/Phasmaphobia_Semi.fbx", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_LIGHT);
		this->_model3->upload();
		// -----

		// Text test ----
		{
			this->_text->addText(this->_font, "TEXTURES + LIGHT", {-10.F, 2.0F, 0});
			this->_text->addText(this->_font, "NONE", {10.F, 2.0F, 0});
			this->_text->addText(this->_font, "TEXTURES", {0.F, 2.0F, 0});
			this->_text->upload();
		}
		// ------
	}

	void Game::shutdown() {
		this->_render = nullptr;
		this->_camera = nullptr;

		this->_model = nullptr;
		this->_model2 = nullptr;
		this->_model3 = nullptr;

		this->_text = nullptr;

		rawrbox::LightManager::get().destroy();
		rawrbox::Engine::shutdown();
	}

	void Game::pollEvents() {
		if (this->_window == nullptr) return;
		this->_window->pollEvents();
	}

	void Game::update(float deltaTime, int64_t gameTime) {
		if (this->_camera == nullptr) return;
		this->_camera->update(deltaTime);
	}

	void Game::drawWorld() {
		if (this->_model == nullptr || this->_model2 == nullptr || this->_model3 == nullptr || this->_text == nullptr) return;
		auto pos = this->_camera->getPos();

		this->_model->draw(pos);
		this->_model2->draw(pos);
		this->_model3->draw(pos);

		this->_text->draw(pos);
	}

	void printFrames() {
		int64_t now = bx::getHPCounter();
		static int64_t last = now;
		const int64_t frameTime = now - last;
		last = now;

		const auto freq = static_cast<double>(bx::getHPFrequency());
		const double toMs = 1000.0 / freq;

		bgfx::dbgTextPrintf(1, 4, 0x0f, "Frame: %7.3f[ms]", double(frameTime) * toMs);
	}

	void Game::draw() {
		if (this->_render == nullptr) return;
		this->_render->swapBuffer(); // Clean up and set renderer

		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, this->_camera->getViewMtx().data(), this->_camera->getProjMtx().data());

		// DEBUG ----
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(1, 1, 0x1f, "003-assimp-loading");
		bgfx::dbgTextPrintf(1, 2, 0x3f, "Description: ASSIMP model loading test");
		printFrames();
		// -----------

		this->drawWorld();

		this->_render->render(true); // Commit primitives
	}
} // namespace assimp
