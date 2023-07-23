
#include <rawrbox/render/camera/orbital.hpp>
#include <rawrbox/render/gizmos.hpp>
#include <rawrbox/render/model/assimp/assimp_importer.hpp>
#include <rawrbox/render/resources/assimp/model.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>

#include <light/game.hpp>

#include <bx/bx.h>
#include <bx/math.h>

#include <vector>

namespace light {

	void Game::setupGLFW() {
		this->_window = std::make_unique<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("LIGHT TEST");
		this->_window->setRenderer(
		    bgfx::RendererType::Count, []() {}, [this]() { this->drawWorld(); });
		this->_window->create(1024, 768, rawrbox::WindowFlags::Debug::TEXT | rawrbox::WindowFlags::Debug::PROFILER | rawrbox::WindowFlags::Window::WINDOWED | rawrbox::WindowFlags::Features::MULTI_THREADED);
		this->_window->onWindowClose += [this](auto& w) { this->shutdown(); };
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
		rawrbox::RESOURCES::addLoader<rawrbox::AssimpLoader>();

		// Setup binds ---
		this->_window->onKey += [](rawrbox::Window& w, uint32_t key, uint32_t scancode, uint32_t action, uint32_t mods) {
			if (action != KEY_ACTION_UP) return;

			if (key == KEY_F1) rawrbox::RENDERER_DEBUG = rawrbox::RENDER_DEBUG_MODE::DEBUG_OFF;
			if (key == KEY_F2) rawrbox::RENDERER_DEBUG = rawrbox::RENDER_DEBUG_MODE::DEBUG_CLUSTER_Z;
			if (key == KEY_F3) rawrbox::RENDERER_DEBUG = rawrbox::RENDER_DEBUG_MODE::DEBUG_CLUSTER_COUNT;
		};
		// ----------

		// Load content ---
		this->loadContent();
		// -----
	}

	void Game::loadContent() {
		std::array<std::pair<std::string, uint32_t>, 2> initialContentFiles = {
		    std::make_pair<std::string, uint32_t>("cour.ttf", 0),
		    std::make_pair<std::string, uint32_t>("content/models/light_test/test.fbx", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_LIGHT | rawrbox::ModelLoadFlags::Debug::PRINT_MATERIALS)};

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
		this->_font = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>("cour.ttf")->getSize(24);

		// Assimp test ---
		auto mdl = rawrbox::RESOURCES::getFile<rawrbox::ResourceAssimp>("./content/models/light_test/test.fbx")->get();

		this->_model->load(*mdl);
		this->_model->setPos({0, 0, 0});
		//   -----

		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) return;
		this->_model.reset();

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

		if (this->_ready) {
			this->_sunDir = {std::cos(rawrbox::BGFX_FRAME * 0.01F) * 1.F, 1.F, std::sin(rawrbox::BGFX_FRAME * 0.01F) * 1.F};
			rawrbox::LIGHTS::setSun(this->_sunDir, {0.2F, 0.2F, 0.2F, 1.F});

			auto light = rawrbox::LIGHTS::getLight(0);
			if (light != nullptr) {
				light->setOffsetPos({0, std::cos(rawrbox::BGFX_FRAME * 0.01F) * 1.F, 0});
			}
		}
	}

	void Game::drawWorld() {
		if (!this->_ready || this->_model == nullptr) return;
		this->_model->draw();
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
		bgfx::dbgTextPrintf(1, 1, 0x1f, "004-light-support");
		bgfx::dbgTextPrintf(1, 2, 0x3f, "Description: Light test");
		printFrames();
		// -----------

		if (!this->_ready) {
			bgfx::dbgTextPrintf(1, 11, 0x70, "                                   ");
			bgfx::dbgTextPrintf(1, 12, 0x70, "          LOADING CONTENT          ");
			bgfx::dbgTextPrintf(1, 13, 0x70, "                                   ");
		} else {
			bgfx::dbgTextPrintf(1, 11, 0x4f, "F1 to hide cluster debug");
			bgfx::dbgTextPrintf(1, 12, 0x4f, "F2 to show z cluster debug");
			bgfx::dbgTextPrintf(1, 13, 0x4f, "F3 to show cluster light debug");

			bgfx::dbgTextPrintf(1, 15, 0x2f, fmt::format("SUN ANGLE: {},{},{}", this->_sunDir.x, this->_sunDir.y, this->_sunDir.z).c_str());
		}

		this->_window->render(); // Draw world & commit primitives
	}
} // namespace light
