

#include <rawrbox/render/camera/orbital.hpp>
#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/model/utils/mesh.hpp>
#include <rawrbox/render/resources/assimp/model.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/resources/manager.hpp>

#include <decal_test/game.hpp>

#include <fmt/format.h>

#include <random>
#include <vector>

namespace decal_test {
	void Game::setupGLFW() {
		this->_window = std::make_unique<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("DECALS TEST");
		this->_window->setRenderer<>(
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

		// Setup loaders
		rawrbox::RESOURCES::addLoader<rawrbox::TextureLoader>();
		rawrbox::RESOURCES::addLoader<rawrbox::AssimpLoader>();

		// Load content ---
		this->loadContent();
		// -----
	}

	void Game::loadContent() {
		std::array<std::pair<std::string, uint32_t>, 2> initialContentFiles = {
		    std::make_pair<std::string, uint32_t>("content/textures/decals.png", 64),
		    std::make_pair<std::string, uint32_t>("content/models/decal_test/test.fbx", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_LIGHT)};

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
		rawrbox::DECALS::setAtlasTexture(rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./content/textures/decals.png")->get());

		std::random_device prng;
		std::uniform_int_distribution<int> dist(0, 4);
		std::uniform_real_distribution<float> distRot(-1.5F, 1.5F);
		std::uniform_real_distribution<float> a(0.F, 0.1F);

		float x = -3.F;
		for (int i = 0; i < 30; i++) {
			rawrbox::DECALS::add({distRot(prng) + x, a(prng), distRot(prng) - 1.55F}, 90, rawrbox::Colors::Green, dist(prng));
			rawrbox::DECALS::add({distRot(prng) + x, distRot(prng) + 1.25F, 0.F}, 0, rawrbox::Colors::Red, dist(prng));
		}

		x = 3.F;
		for (int i = 0; i < 30; i++) {
			rawrbox::DECALS::add({distRot(prng) + x, a(prng), distRot(prng) - 1.55F}, 90, rawrbox::Colors::Green, dist(prng));
			rawrbox::DECALS::add({distRot(prng) + x, distRot(prng) + 1.25F, 0.F}, 0, rawrbox::Colors::Red, dist(prng));
		}

		// Setup

		// Assimp test ---
		auto mdl = rawrbox::RESOURCES::getFile<rawrbox::ResourceAssimp>("./content/models/decal_test/test.fbx")->get();

		this->_model2->load(*mdl);
		this->_model2->setRecieveDecals(true);
		this->_model2->setPos({-3, 0, 0});

		this->_model3->load(*mdl);
		this->_model3->setRecieveDecals(true);
		this->_model3->setPos({3, 0, 0});
		//   -----

		this->_model->setOptimizable(false);

		{
			auto mesh = rawrbox::MeshUtils::generateSphere({-3.F, 0.F, -1.F}, 0.5F);
			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateSphere({3.F, 0.F, -1.F}, 0.5F);
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
		this->_model.reset();

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

		if (this->_ready && this->_model != nullptr) {
			this->_model->getMesh()->setPos({std::sin(rawrbox::BGFX_FRAME * 0.01F) * 0.5F - 1.F, -0.05F, -0.55F - std::cos(rawrbox::BGFX_FRAME * 0.01F) * 0.5F});
			this->_model->getMesh(1)->setPos({std::sin(rawrbox::BGFX_FRAME * 0.01F) * 0.5F + 1.F, -0.05F, -0.55F - std::cos(rawrbox::BGFX_FRAME * 0.01F) * 0.5F});
		}
	}

	void Game::printFrames() {
		const bgfx::Stats* stats = bgfx::getStats();

		bgfx::dbgTextPrintf(1, 4, 0x6f, "GPU %0.6f [ms]", double(stats->gpuTimeEnd - stats->gpuTimeBegin) * 1000.0 / stats->gpuTimerFreq);
		bgfx::dbgTextPrintf(1, 5, 0x6f, "CPU %0.6f [ms]", double(stats->cpuTimeEnd - stats->cpuTimeBegin) * 1000.0 / stats->cpuTimerFreq);
		bgfx::dbgTextPrintf(1, 7, 0x5f, fmt::format("TRIANGLES: {}", stats->numPrims[bgfx::Topology::TriList]).c_str());
		bgfx::dbgTextPrintf(1, 8, 0x5f, fmt::format("DRAW CALLS: {}", stats->numDraw).c_str());
		bgfx::dbgTextPrintf(1, 9, 0x5f, fmt::format("COMPUTE CALLS: {}", stats->numCompute).c_str());

		bgfx::dbgTextPrintf(1, 11, 0x5f, fmt::format("TOTAL DECALS: {}", rawrbox::DECALS::count()).c_str());
	}
	void Game::drawWorld() {
		if (!this->_ready) return;

		this->_model->draw();
		this->_model2->draw();
		this->_model3->draw();
	}

	void Game::draw() {
		if (this->_window == nullptr) return;

		// DEBUG ----
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(1, 1, 0x1f, "011-decals-test");
		bgfx::dbgTextPrintf(1, 2, 0x3f, "Description: DECALS test");
		this->printFrames();
		// -----------

		if (!this->_ready) {
			bgfx::dbgTextPrintf(1, 10, 0x70, "                                   ");
			bgfx::dbgTextPrintf(1, 11, 0x70, "          LOADING CONTENT          ");
			bgfx::dbgTextPrintf(1, 12, 0x70, "                                   ");
		}

		this->_window->render(); // Commit primitives
	}
} // namespace decal_test