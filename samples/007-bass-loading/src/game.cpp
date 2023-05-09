
#include <rawrbox/bass/manager.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/utils/keys.hpp>

#include <bass_test/game.hpp>

#include <bx/bx.h>
#include <bx/math.h>
#include <bx/timer.h>
#include <fmt/printf.h>

#include <vector>

namespace bass_test {
	void Game::init() {
		int width = 1024;
		int height = 768;

		this->_window = std::make_unique<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("BASS TEST");
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
		this->_render->setClearColor(0x00000000);

		// Setup camera
		this->_camera = std::make_shared<rawrbox::CameraOrbital>(this->_window.get());
		this->_camera->setPos({0.F, 5.F, -5.F});
		this->_camera->setAngle({0.F, bx::toRad(-45), 0.F, 0.F});
		// --------------

		rawrbox::SoundManager::get().initialize();
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

		// SOUND -----
		// https://i.rawr.dev/Mystery%20Skulls%20-%20Freaking%20Out.mp3
		// https://i.rawr.dev/Just_a_Bit_Crazy.ogg
		this->_sound = rawrbox::SoundManager::get().loadHTTPSound("https://i.rawr.dev/Just_a_Bit_Crazy.ogg", rawrbox::SoundFlags::SOUND_3D | rawrbox::SoundFlags::BEAT_DETECTION | rawrbox::SoundFlags::BPM_DETECTION)->createInstance();
		this->_sound->setVolume(1.F);
		this->_sound->setLooping(true);
		this->_sound->set3D(10.F);
		this->_sound->setPosition({-3.F, 1.F, 0});
		this->_sound->setTempo(0.8F);
		this->_sound->play();

		// this->_sound->setBeatSettings(8, 8, 2.f);
		this->_sound->onBEAT += [this](double pos) {
			this->_beat = 0.5F;
		};

		this->_sound->onBPM += [](float bpm) {
			fmt::print("BPM: {}\n", bpm);
		};

		this->_sound2 = rawrbox::SoundManager::get().loadSound("./content/sounds/clownmusic.ogg", rawrbox::SoundFlags::SOUND_3D)->createInstance();
		this->_sound2->setLooping(true);
		this->_sound2->set3D(10.F);
		this->_sound2->setPosition({3.F, 1.F, 0});
		this->_sound2->setTempo(1.2F);
		this->_sound2->play();

		// --------

		// Text test ----
		{
			this->_beatText->addText(this->_font, "BEAT", {-3.F, 1.3F, 0});
			this->_beatText->upload();
		}

		{
			this->_text->addText(this->_font, "HTTP LOADING", {-3.F, 1.1F, 0});
			this->_text->addText(this->_font, "LOCAL LOADING", {3.F, 1.1F, 0});
			this->_text->upload();
		}
		// ------

		// GRID -----
		{
			auto mesh = this->_modelGrid->generateGrid(12, {0.F, 0.F, 0.F});
			this->_modelGrid->addMesh(mesh);
			this->_modelGrid->upload();
		}
	}

	void Game::shutdown() {
		this->_render = nullptr;
		this->_camera = nullptr;

		this->_sound = nullptr;
		this->_modelGrid = nullptr;
		this->_beatText = nullptr;
		this->_text = nullptr;

		rawrbox::SoundManager::get().shutdown();
		rawrbox::Engine::shutdown();
	}

	void Game::pollEvents() {
		if (this->_window == nullptr) return;
		this->_window->pollEvents();
	}

	void Game::update() {
		if (this->_camera == nullptr) return;
		this->_camera->update();

		rawrbox::SoundManager::get().setListenerLocation(this->_camera->getPos(), this->_camera->getForward(), this->_camera->getUp());
		if (this->_beat > 0.F) this->_beat -= 0.05F;
	}

	void Game::drawWorld() {
		this->_modelGrid->draw({});
		this->_beatText->setPos({0, this->_beat, 0});
		this->_beatText->draw({});
		this->_text->draw({});
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

		// DEBUG ----
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(1, 1, 0x1f, "007-bass-loading");
		bgfx::dbgTextPrintf(1, 2, 0x3f, "Description: BASS test");
		printFrames();
		// -----------

		this->drawWorld();

		this->_render->render(true); // Commit primitives
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, this->_camera->getViewMtx().data(), this->_camera->getProjMtx().data());
	}
} // namespace bass_test
