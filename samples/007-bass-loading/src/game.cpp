
#include <rawrbox/bass/static.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/utils/keys.hpp>

#include <bass_test/game.hpp>

#include <bx/bx.h>
#include <bx/math.h>
#include <fmt/printf.h>

#include <vector>

namespace bass_test {
	void Game::init() {
		int width = 1024;
		int height = 768;

		this->_window = std::make_shared<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("BASS TEST");
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

		rawrbox::BASS.initialize();
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

		// SOUND -----
		// https://i.rawr.dev/Mystery%20Skulls%20-%20Freaking%20Out.mp3
		// https://i.rawr.dev/Just_a_Bit_Crazy.ogg
		this->_sound = rawrbox::BASS.loadHTTPSound("https://i.rawr.dev/Just_a_Bit_Crazy.ogg", rawrbox::SoundFlags::SOUND_3D | rawrbox::SoundFlags::BEAT_DETECTION | rawrbox::SoundFlags::BPM_DETECTION)->createInstance();
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

		this->_sound2 = rawrbox::BASS.loadSound("./content/sounds/clownmusic.ogg", rawrbox::SoundFlags::SOUND_3D)->createInstance();
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
		this->_window = nullptr;
		this->_camera = nullptr;

		this->_sound = nullptr;
		this->_modelGrid = nullptr;
		this->_beatText = nullptr;
		this->_text = nullptr;

		rawrbox::BASS.shutdown();
		rawrbox::Engine::shutdown();
	}

	void Game::pollEvents() {
		if (this->_window == nullptr) return;
		this->_window->pollEvents();
	}

	void Game::update() {
		if (this->_camera == nullptr) return;
		this->_camera->update();

		rawrbox::BASS.setListenerLocation(this->_camera->getPos(), this->_camera->getForward(), this->_camera->getUp());
		if (this->_beat > 0.F) this->_beat -= 0.05F;
	}

	void Game::drawWorld() {
		this->_modelGrid->draw({});
		this->_beatText->setPos({0, this->_beat, 0});
		this->_beatText->draw({});
		this->_text->draw({});
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
		bgfx::dbgTextPrintf(1, 1, 0x1f, "007-bass-loading");
		bgfx::dbgTextPrintf(1, 2, 0x3f, "Description: BASS test");
		printFrames();
		// -----------

		this->drawWorld();

		this->_window->frame(true); // Commit primitives
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, this->_camera->getViewMtx().data(), this->_camera->getProjMtx().data());
	}
} // namespace bass_test
