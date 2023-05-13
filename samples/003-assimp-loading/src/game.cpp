#include <rawrbox/render/model/light/manager.hpp>
#include <rawrbox/utils/keys.hpp>

#include <assimp/game.hpp>

#include <bx/bx.h>
#include <bx/math.h>

#include <vector>

namespace assimp {
	void Game::init() {
		int width = 1024;
		int height = 768;

		this->_window = std::make_shared<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("ASSIMP TEST");
		this->_window->setRenderer(bgfx::RendererType::Count);
		this->_window->onWindowClose += [this](auto& w) {
			this->shutdown();
		};

		this->_window->initialize(width, height, rawrbox::WindowFlags::Debug::TEXT | rawrbox::WindowFlags::Debug::PROFILER | rawrbox::WindowFlags::Window::WINDOWED);

		// Setup camera
		this->_camera = std::make_shared<rawrbox::CameraOrbital>(this->_window);
		this->_camera->setPos({0.F, 5.F, -5.F});
		this->_camera->setAngle({0.F, bx::toRad(-45), 0.F, 0.F});
		// --------------

		this->_textEngine = std::make_unique<rawrbox::TextEngine>();

		// Load content ---
		this->loadContent();
		// -----
	}

	void Game::loadContent() {
		this->_window->upload();

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
		this->_window = nullptr;
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

	void Game::update() {
		if (this->_camera == nullptr) return;
		this->_camera->update();
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
		const bgfx::Stats* stats = bgfx::getStats();

		bgfx::dbgTextPrintf(1, 4, 0x6f, "GPU %0.6f [ms]", double(stats->gpuTimeEnd - stats->gpuTimeBegin) * 1000.0 / stats->gpuTimerFreq);
		bgfx::dbgTextPrintf(1, 5, 0x6f, "CPU %0.6f [ms]", double(stats->cpuTimeEnd - stats->cpuTimeBegin) * 1000.0 / stats->cpuTimerFreq);
		bgfx::dbgTextPrintf(1, 6, 0x6f, fmt::format("TRIANGLES: {} ----->    DRAW CALLS: {}", stats->numPrims[bgfx::Topology::TriList], stats->numDraw).c_str());
	}

	void Game::draw() {
		if (this->_window == nullptr) return;
		this->_window->clear(); // Clean up and set renderer

		// DEBUG ----
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(1, 1, 0x1f, "003-assimp-loading");
		bgfx::dbgTextPrintf(1, 2, 0x3f, "Description: ASSIMP model loading test");
		printFrames();
		// -----------

		this->drawWorld();

		this->_window->frame(true); // Commit primitives
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, this->_camera->getViewMtx().data(), this->_camera->getProjMtx().data());
	}
} // namespace assimp
