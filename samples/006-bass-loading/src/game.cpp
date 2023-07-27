
#include <rawrbox/bass/manager.hpp>
#include <rawrbox/bass/resources/sound.hpp>
#include <rawrbox/render/camera/orbital.hpp>
#include <rawrbox/render/gizmos.hpp>
#include <rawrbox/render/model/utils/mesh.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>

#include <bass_test/game.hpp>

#include <bx/bx.h>
#include <bx/math.h>
#include <fmt/printf.h>

#include <vector>

namespace bass_test {
	void Game::setupGLFW() {
		this->_window = std::make_unique<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("BASS TEST");
		this->_window->setRenderer<>(
		    bgfx::RendererType::Count, []() {}, [this]() { this->drawWorld(); });
		this->_window->create(1024, 768, rawrbox::WindowFlags::Debug::TEXT | rawrbox::WindowFlags::Debug::PROFILER | rawrbox::WindowFlags::Window::WINDOWED | rawrbox::WindowFlags::Features::MULTI_THREADED);
		this->_window->onWindowClose += [this](auto& /*w*/) { this->shutdown(); };
	}

	void Game::init() {
		if (this->_window == nullptr) return;
		this->_window->initializeBGFX();

		// Setup camera
		auto cam = this->_window->setupCamera<rawrbox::CameraOrbital>(*this->_window);
		cam->setPos({0.F, 5.F, -5.F});
		cam->setAngle({0.F, bx::toRad(-45), 0.F, 0.F});
		// --------------

		rawrbox::RESOURCES::addLoader<rawrbox::FontLoader>();
		rawrbox::RESOURCES::addLoader<rawrbox::BASSLoader>();

		// Load content ---
		this->loadContent();
		// -----
	}

	void Game::loadContent() {
		std::array initialContentFiles = {
		    std::make_pair<std::string, uint32_t>("cour.ttf", 0),
		    std::make_pair<std::string, uint32_t>("content/sounds/clownmusic.ogg", 0 | rawrbox::SoundFlags::SOUND_3D)};

		for (auto& f : initialContentFiles) {
			this->_loadingFiles++;

			rawrbox::RESOURCES::loadFileAsync(f.first, f.second, [this]() {
				this->_loadingFiles--;
				if (this->_loadingFiles <= 0) {
					rawrbox::runOnRenderThread([this]() { this->contentLoaded(); });
				}
			});
		}

		this->_window->upload();
	}

	void Game::contentLoaded() {
		// Fonts -----
		this->_font = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>("cour.ttf")->getSize(24);
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

		this->_sound->onBEAT += [this](double /*pos*/) {
			this->_beat = 0.5F;
		};

		this->_sound->onBPM += [](float bpm) {
			fmt::print("BPM: {}\n", bpm);
		};

		this->_sound2 = rawrbox::RESOURCES::getFile<rawrbox::ResourceBASS>("content/sounds/clownmusic.ogg")->get()->createInstance();
		this->_sound2->setLooping(true);
		this->_sound2->set3D(10.F);
		this->_sound2->setPosition({3.F, 1.F, 0});
		this->_sound2->setTempo(1.2F);
		this->_sound2->play();

		// Text test ----
		{
			this->_beatText->addText(*this->_font, "BEAT", {-3.F, 1.3F, 0});
			this->_beatText->upload();
		}

		{
			this->_text->addText(*this->_font, "HTTP LOADING", {-3.F, 1.1F, 0});
			this->_text->addText(*this->_font, "LOCAL LOADING", {3.F, 1.1F, 0});
			this->_text->upload();
		}
		// ------

		// GRID -----
		{
			auto mesh = rawrbox::MeshUtils::generateGrid(12, {0.F, 0.F, 0.F});
			this->_modelGrid->addMesh(mesh);
			this->_modelGrid->upload();
		}

		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) return;
		this->_sound.reset();
		this->_sound2.reset();

		this->_modelGrid.reset();
		this->_beatText.reset();
		this->_text.reset();

		rawrbox::GIZMOS::shutdown();
		rawrbox::RESOURCES::shutdown();
		rawrbox::ASYNC::shutdown();

		this->_window->unblockPoll();
		this->_window.reset();
	}

	void Game::pollEvents() {
		if (this->_window == nullptr) return;
		this->_window->pollEvents();
	}

	void Game::update() {
		if (this->_window == nullptr) return;
		this->_window->update();

		auto cam = rawrbox::MAIN_CAMERA;
		rawrbox::BASS::setListenerLocation(cam->getPos(), cam->getForward(), cam->getUp());

		if (this->_beat > 0.F) this->_beat -= 0.05F;
		this->_beatText->setPos({0, this->_beat, 0});
	}

	void Game::drawWorld() {
		if (!this->_ready) return;
		this->_modelGrid->draw();
		this->_beatText->draw();
		this->_text->draw();
	}

	void Game::printFrames() {
		const bgfx::Stats* stats = bgfx::getStats();

		bgfx::dbgTextPrintf(1, 4, 0x6f, "GPU %0.6f [ms]", double(stats->gpuTimeEnd - stats->gpuTimeBegin) * 1000.0 / stats->gpuTimerFreq);
		bgfx::dbgTextPrintf(1, 5, 0x6f, "CPU %0.6f [ms]", double(stats->cpuTimeEnd - stats->cpuTimeBegin) * 1000.0 / stats->cpuTimerFreq);
		bgfx::dbgTextPrintf(1, 7, 0x5f, fmt::format("TRIANGLES: {}", stats->numPrims[bgfx::Topology::TriList]).c_str());
		bgfx::dbgTextPrintf(1, 8, 0x5f, fmt::format("DRAW CALLS: {}", stats->numDraw).c_str());
		bgfx::dbgTextPrintf(1, 9, 0x5f, fmt::format("COMPUTE CALLS: {}", stats->numCompute).c_str());
	}

	void Game::draw() {
		if (this->_window == nullptr) return;

		// DEBUG ----
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(1, 1, 0x1f, "006-bass-loading");
		bgfx::dbgTextPrintf(1, 2, 0x3f, "Description: BASS test");
		printFrames();
		// -----------

		if (!this->_ready) {
			bgfx::dbgTextPrintf(1, 10, 0x70, "                                   ");
			bgfx::dbgTextPrintf(1, 11, 0x70, "          LOADING CONTENT          ");
			bgfx::dbgTextPrintf(1, 12, 0x70, "                                   ");
		}

		// Draw DEBUG ---
		rawrbox::GIZMOS::draw();
		// -----------

		this->_window->render(); // Commit primitives
	}
} // namespace bass_test
