
#include <rawrbox/render/camera/orbital.hpp>
#include <rawrbox/render/model/utils/mesh.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/webm/resources/webm.hpp>

#include <webm_test/game.hpp>

#include <fmt/format.h>

namespace webm_test {
	void Game::setupGLFW() {
		this->_window = std::make_unique<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("WEBM TEST");
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

		// Setup loaders
		rawrbox::RESOURCES::addLoader<rawrbox::TextureLoader>();
		rawrbox::RESOURCES::addLoader<rawrbox::WEBMLoader>();
		// ----------

		// Load content ---
		this->loadContent();
		//   -----
	}

	void Game::loadContent() {
		std::array initialContentFiles = {
		    std::make_pair<std::string, uint32_t>("./content/video/webm_test.webm", 0)};

		this->_loadingFiles = static_cast<int>(initialContentFiles.size());
		for (auto& f : initialContentFiles) {
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
		auto tex = rawrbox::RESOURCES::getFile<rawrbox::ResourceWEBM>("./content/video/webm_test.webm")->get();
		this->_model->setOptimizable(false);

		{
			auto mesh = rawrbox::MeshUtils::generatePlane({0.F, 4.0F, 0.F}, {4.F, 3.F});
			mesh.setTexture(tex);
			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateGrid(12, {0.F, 0.F, 0.F});
			this->_model->addMesh(mesh);
		}
		// ----

		this->_model->upload();
		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) return;

		rawrbox::RESOURCES::shutdown();
		rawrbox::ASYNC::shutdown();

		this->_model.reset();

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

	void Game::printFrames() {
		const bgfx::Stats* stats = bgfx::getStats();

		bgfx::dbgTextPrintf(1, 4, 0x6f, "GPU %0.6f [ms]", double(stats->gpuTimeEnd - stats->gpuTimeBegin) * 1000.0 / stats->gpuTimerFreq);
		bgfx::dbgTextPrintf(1, 5, 0x6f, "CPU %0.6f [ms]", double(stats->cpuTimeEnd - stats->cpuTimeBegin) * 1000.0 / stats->cpuTimerFreq);
		bgfx::dbgTextPrintf(1, 7, 0x5f, fmt::format("TRIANGLES: {}", stats->numPrims[bgfx::Topology::TriList]).c_str());
		bgfx::dbgTextPrintf(1, 8, 0x5f, fmt::format("DRAW CALLS: {}", stats->numDraw).c_str());
		bgfx::dbgTextPrintf(1, 9, 0x5f, fmt::format("COMPUTE CALLS: {}", stats->numCompute).c_str());
	}
	void Game::drawWorld() {
		if (!this->_ready) return;
		if (this->_model != nullptr) this->_model->draw();
	}

	void Game::draw() {
		if (this->_window == nullptr) return;

		// DEBUG ----
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(1, 1, 0x1f, "012-webm");
		bgfx::dbgTextPrintf(1, 2, 0x3f, "Description: WEBM test");
		this->printFrames();
		// -----------

		if (!this->_ready) {
			bgfx::dbgTextPrintf(1, 10, 0x70, "                                   ");
			bgfx::dbgTextPrintf(1, 11, 0x70, "          LOADING CONTENT          ");
			bgfx::dbgTextPrintf(1, 12, 0x70, "                                   ");
		}

		this->_window->render(); // Commit primitives
	}
} // namespace webm_test
