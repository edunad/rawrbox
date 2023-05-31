
#include <rawrbox/bass/resources/sound.hpp>
#include <rawrbox/debug/gizmos.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/resources/manager.hpp>
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

		rawrbox::RESOURCES::addLoader(std::make_unique<rawrbox::FontLoader>());
		rawrbox::RESOURCES::addLoader(std::make_unique<rawrbox::BASSLoader>());

		// Load content ---
		this->loadContent();
		// -----
	}

	void Game::loadContent() {

		std::array initialContentFiles = {
		    std::make_pair<std::string, uint32_t>("cour.ttf", 0),
		    std::make_pair<std::string, uint32_t>("content/sounds/clownmusic.ogg", 0 | rawrbox::SoundFlags::SOUND_3D)};

		rawrbox::ASYNC::run([initialContentFiles]() {
			for (auto& f : initialContentFiles) {
				rawrbox::RESOURCES::loadFile(f.first, f.second);
			} }, [this] { rawrbox::runOnMainThread([this]() {
										  rawrbox::RESOURCES::upload();
										  this->contentLoaded();
									  }); });

		this->_window->upload();

		// DEBUG ---
		rawrbox::GIZMOS::upload();
		// -----------
	}

	void Game::contentLoaded() {
		// Fonts -----
		this->_font = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>("cour.ttf")->getSize(16);
		//  ------

		// SOUND -----
		// https://i.rawr.dev/Mystery%20Skulls%20-%20Freaking%20Out.mp3
		// https://i.rawr.dev/Just_a_Bit_Crazy.ogg
		this->_sound = rawrbox::BASS::loadHTTPSound("https://i.rawr.dev/Just_a_Bit_Crazy.ogg", rawrbox::SoundFlags::SOUND_3D | rawrbox::SoundFlags::BEAT_DETECTION | rawrbox::SoundFlags::BPM_DETECTION)->createInstance();
		this->_sound->setVolume(1.F);
		this->_sound->setLooping(true);
		this->_sound->set3D(10.F);
		this->_sound->setPosition({-3.F, 1.F, 0});
		this->_sound->setTempo(0.8F);
		this->_sound->play();

		this->_sound->onBEAT += [this](double pos) {
			this->_beat = 0.5F;
		};

		this->_sound->onBPM += [](float bpm) {
			fmt::print("BPM: {}\n", bpm);
		};

		this->_sound2 = rawrbox::RESOURCES::getFile<rawrbox::ResourceBASS>("content/sounds/clownmusic.ogg")->sound->createInstance();
		this->_sound2->setLooping(true);
		this->_sound2->set3D(10.F);
		this->_sound2->setPosition({3.F, 1.F, 0});
		this->_sound2->setTempo(1.2F);
		this->_sound2->play();

		// Text test ----
		auto f = this->_font.lock();
		{
			this->_beatText->addText(f, "BEAT", {-3.F, 1.3F, 0});
			this->_beatText->upload();
		}

		{
			this->_text->addText(f, "HTTP LOADING", {-3.F, 1.1F, 0});
			this->_text->addText(f, "LOCAL LOADING", {3.F, 1.1F, 0});
			this->_text->upload();
		}
		// ------

		// GRID -----
		{
			auto mesh = this->_modelGrid->generateGrid(12, {0.F, 0.F, 0.F});
			this->_modelGrid->addMesh(mesh);
			this->_modelGrid->upload();
		}

		this->_ready = true;
	}

	void Game::shutdown() {
		this->_window = nullptr;
		this->_camera = nullptr;

		this->_sound = nullptr;
		this->_modelGrid = nullptr;
		this->_beatText = nullptr;
		this->_text = nullptr;

		rawrbox::GIZMOS::shutdown();
		rawrbox::RESOURCES::shutdown();
		rawrbox::LIGHTS::shutdown();
		rawrbox::Engine::shutdown();
	}

	void Game::pollEvents() {
		if (this->_window == nullptr) return;
		this->_window->pollEvents();
	}

	void Game::update() {
		if (this->_camera == nullptr) return;
		this->_camera->update();

		rawrbox::BASS::setListenerLocation(this->_camera->getPos(), this->_camera->getForward(), this->_camera->getUp());
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
		bgfx::dbgTextPrintf(1, 1, 0x1f, "006-bass-loading");
		bgfx::dbgTextPrintf(1, 2, 0x3f, "Description: BASS test");
		printFrames();
		// -----------

		if (this->_ready) {
			this->drawWorld();
		} else {
			bgfx::dbgTextPrintf(1, 10, 0x70, "                                   ");
			bgfx::dbgTextPrintf(1, 11, 0x70, "          LOADING CONTENT          ");
			bgfx::dbgTextPrintf(1, 12, 0x70, "                                   ");
		}

		// Draw DEBUG ---
		rawrbox::GIZMOS::draw();
		// -----------

		this->_window->frame(); // Commit primitives
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, this->_camera->getViewMtx().data(), this->_camera->getProjMtx().data());
	}
} // namespace bass_test
