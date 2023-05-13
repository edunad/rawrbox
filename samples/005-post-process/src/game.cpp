
#include <rawrbox/render/model/light/manager.hpp>
#include <rawrbox/render/model/mesh.hpp>
#include <rawrbox/render/postprocess/bloom.hpp>
#include <rawrbox/render/postprocess/dither_psx.hpp>
#include <rawrbox/render/postprocess/static_noise.hpp>
#include <rawrbox/utils/keys.hpp>

#include <post_process/game.hpp>

#include <bx/bx.h>
#include <bx/math.h>

#include <vector>

namespace post_process {
	void Game::init() {
		int width = 1024;
		int height = 768;

		this->_window = std::make_shared<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("POST-PROCESS TEST");
		this->_window->setRenderer(bgfx::RendererType::Count);
		this->_window->onWindowClose += [this](auto& w) {
			this->shutdown();
		};

		this->_window->initialize(width, height, rawrbox::WindowFlags::Window::WINDOWED | rawrbox::WindowFlags::Debug::PROFILER | rawrbox::WindowFlags::Debug::TEXT);

		this->_postProcess = std::make_shared<rawrbox::PostProcessManager>(this->_window->getSize());
		this->_postProcess->add(std::make_shared<rawrbox::PostProcessBloom>(0.015F));
		this->_postProcess->add(std::make_shared<rawrbox::PostProcessPSXDither>(rawrbox::DITHER_SIZE::SLOW_MODE));
		this->_postProcess->add(std::make_shared<rawrbox::PostProcessStaticNoise>(0.1F));

		// Setup camera
		this->_camera = std::make_shared<rawrbox::CameraOrbital>(this->_window);
		this->_camera->setPos({0.F, 5.F, -5.F});
		this->_camera->setAngle({0.F, bx::toRad(-45), 0.F, 0.F});
		// --------------

		// Load content ---
		this->loadContent();
		// -----
	}

	void Game::loadContent() {
		this->_window->upload();
		this->_postProcess->upload();

		// Assimp test ---
		this->_model->load("./content/models/ps1_road/scene.gltf", rawrbox::ModelLoadFlags::IMPORT_TEXTURES);
		this->_model->setScale({0.01F, 0.01F, 0.01F});
		this->_model->upload();
		// -----
	}

	void Game::shutdown() {
		this->_window = nullptr;
		this->_camera = nullptr;
		this->_model = nullptr;

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
		if (this->_model == nullptr) return;
		this->_model->draw(this->_camera->getPos());

		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, this->_camera->getViewMtx().data(), this->_camera->getProjMtx().data());
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
		bgfx::dbgTextPrintf(1, 1, 0x1f, "005-post-process");
		bgfx::dbgTextPrintf(1, 2, 0x3f, "Description: Post-processing test");
		printFrames();
		// -----------

		this->_postProcess->begin();
		this->drawWorld();
		this->_postProcess->end();

		this->_window->frame(); // Commit primitives
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, this->_camera->getViewMtx().data(), this->_camera->getProjMtx().data());
	}
} // namespace post_process
