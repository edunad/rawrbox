
#include <rawrbox/render/camera/orbital.hpp>
#include <rawrbox/render/model/assimp/assimp_importer.hpp>
#include <rawrbox/render/postprocess/bloom.hpp>
#include <rawrbox/render/postprocess/dither_psx.hpp>
#include <rawrbox/render/postprocess/static_noise.hpp>
#include <rawrbox/render/resources/assimp/model.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>

#include <post_process/game.hpp>

#include <bx/bx.h>
#include <bx/math.h>

#include <vector>

namespace post_process {
	void Game::setupGLFW() {
		this->_window = std::make_unique<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("POST-PROCESS TEST");
		this->_window->setRenderer<rawrbox::RendererBase>(
		    bgfx::RendererType::Count, []() {}, [this]() { this->drawWorld(); });
		this->_window->create(1024, 768, rawrbox::WindowFlags::Debug::TEXT | rawrbox::WindowFlags::Debug::PROFILER | rawrbox::WindowFlags::Window::WINDOWED | rawrbox::WindowFlags::Features::MULTI_THREADED);
		this->_window->onWindowClose += [this](auto& w) { this->shutdown(); };
	}

	void Game::init() {
		if (this->_window == nullptr) return;
		this->_window->initializeBGFX();

		this->_postProcess = std::make_unique<rawrbox::PostProcessManager>(this->_window->getSize());
		this->_postProcess->add<rawrbox::PostProcessBloom>(0.015F);
		this->_postProcess->add<rawrbox::PostProcessPSXDither>(rawrbox::DITHER_SIZE::SLOW_MODE);
		this->_postProcess->add<rawrbox::PostProcessStaticNoise>(0.1F);

		// Setup camera
		auto cam = this->_window->setupCamera<rawrbox::CameraOrbital>(*this->_window);
		cam->setPos({0.F, 5.F, -5.F});
		cam->setAngle({0.F, bx::toRad(-45), 0.F, 0.F});
		// --------------

		rawrbox::RESOURCES::addLoader(std::make_unique<rawrbox::FontLoader>());
		rawrbox::RESOURCES::addLoader(std::make_unique<rawrbox::AssimpLoader>());

		// Load content ---
		this->loadContent();
		// -----
	}

	void Game::loadContent() {
		std::array<std::pair<std::string, uint32_t>, 1> initialContentFiles = {
		    std::make_pair<std::string, uint32_t>("content/models/ps1_road/scene.gltf", 0 | rawrbox::ModelLoadFlags::IMPORT_TEXTURES)};

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
		this->_postProcess->upload();
	}

	void Game::contentLoaded() {
		// Assimp test ---
		auto mdl = rawrbox::RESOURCES::getFile<rawrbox::ResourceAssimp>("./content/models/ps1_road/scene.gltf")->get();

		this->_model->load(*mdl);
		this->_model->setScale({0.01F, 0.01F, 0.01F});
		//   -----

		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) return;
		this->_model.reset();
		this->_postProcess.reset();

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
	}

	void Game::drawWorld() {
		if (!this->_ready || this->_model == nullptr) return;

		// this->_postProcess->begin();
		this->_model->draw();
		// this->_postProcess->end();
	}

	void Game::printFrames() {
		const bgfx::Stats* stats = bgfx::getStats();

		bgfx::dbgTextPrintf(1, 4, 0x6f, "GPU %0.6f [ms]", double(stats->gpuTimeEnd - stats->gpuTimeBegin) * 1000.0 / stats->gpuTimerFreq);
		bgfx::dbgTextPrintf(1, 5, 0x6f, "CPU %0.6f [ms]", double(stats->cpuTimeEnd - stats->cpuTimeBegin) * 1000.0 / stats->cpuTimerFreq);
		bgfx::dbgTextPrintf(1, 6, 0x6f, fmt::format("TRIANGLES: {} ----->    DRAW CALLS: {}", stats->numPrims[bgfx::Topology::TriList], stats->numDraw).c_str());
	}

	void Game::draw() {
		if (this->_window == nullptr) return;

		// DEBUG ----
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(1, 1, 0x1f, "005-post-process");
		bgfx::dbgTextPrintf(1, 2, 0x3f, "Description: Post-processing test");
		printFrames();
		// -----------

		if (!this->_ready) {
			bgfx::dbgTextPrintf(1, 10, 0x70, "                                   ");
			bgfx::dbgTextPrintf(1, 11, 0x70, "          LOADING CONTENT          ");
			bgfx::dbgTextPrintf(1, 12, 0x70, "                                   ");
		}

		this->_window->render(); // Commit primitives
	}
} // namespace post_process
