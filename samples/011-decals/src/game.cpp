

#include <rawrbox/render/camera/orbital.hpp>
#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/light/point.hpp>
#include <rawrbox/render/materials/lit.hpp>
#include <rawrbox/render/model/utils/mesh.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>

#include <decal_test/game.hpp>

#include <fmt/format.h>

#include <random>

namespace decal_test {
	void Game::setupGLFW() {
		this->_window = std::make_unique<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("DECALS TEST");
		this->_window->setRenderer<>(
		    bgfx::RendererType::Count, []() {}, [this]() { this->drawWorld(); });
		this->_window->create(1024, 768, rawrbox::WindowFlags::Debug::TEXT | rawrbox::WindowFlags::Debug::PROFILER | rawrbox::WindowFlags::Window::WINDOWED | rawrbox::WindowFlags::Features::MULTI_THREADED);
		this->_window->onWindowClose += [this](auto& /*w*/) { this->shutdown(); };
		this->_window->onIntroCompleted = [this]() {
			this->loadContent();
		};
	}

	void Game::init() {
		if (this->_window == nullptr) return;

		// Setup camera
		auto cam = this->_window->setupCamera<rawrbox::CameraOrbital>(*this->_window);
		cam->setPos({0.F, 5.F, -5.F});
		cam->setAngle({0.F, bx::toRad(-45), 0.F, 0.F});
		// --------------

		// Setup loaders
		rawrbox::RESOURCES::addLoader<rawrbox::TextureLoader>();
		// ----------

		// Setup materials ---
		this->_model->setMaterial<rawrbox::MaterialLit>();
		// ----

		// Setup binds ---
		this->_window->onKey += [](rawrbox::Window& /*w*/, uint32_t key, uint32_t /*scancode*/, uint32_t action, uint32_t /*mods*/) {
			if (action != KEY_ACTION_UP) return;
			if (key == KEY_F1) {
				rawrbox::RENDERER_DEBUG = rawrbox::RENDERER_DEBUG == rawrbox::RENDER_DEBUG_MODE::DEBUG_OFF ? rawrbox::RENDER_DEBUG_MODE::DEBUG_DECALS : rawrbox::RENDER_DEBUG_MODE::DEBUG_OFF;
			}
		};
		// ----------

		this->_window->initializeBGFX();
	}

	void Game::loadContent() {
		std::array<std::pair<std::string, uint32_t>, 1> initialContentFiles = {
		    std::make_pair<std::string, uint32_t>("content/textures/decals.png", 64)};

		this->_loadingFiles = static_cast<int>(initialContentFiles.size());
		for (auto& f : initialContentFiles) {
			rawrbox::RESOURCES::loadFileAsync(f.first, f.second, [this]() {
				this->_loadingFiles--;
				if (this->_loadingFiles <= 0) {
					rawrbox::runOnRenderThread([this]() { this->contentLoaded(); });
				}
			});
		}
	}

	void Game::contentLoaded() {
		rawrbox::DECALS::setAtlasTexture(rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./content/textures/decals.png")->get());

		std::random_device prng;
		std::uniform_int_distribution<uint16_t> dist(0, 4);
		std::uniform_real_distribution<float> distRot(-1.5F, 1.5F);

		for (int i = 0; i < 30; i++) {
			rawrbox::DECALS::add({distRot(prng), 0.F, distRot(prng) - 1.55F}, {1, 1, 1}, 90, rawrbox::Colors::Green(), dist(prng));
			rawrbox::DECALS::add({distRot(prng), distRot(prng) + 1.25F, 0.F}, {1, 1, 1}, 0, rawrbox::Colors::Red(), dist(prng));
		}

		rawrbox::LIGHTS::addLight<rawrbox::PointLight>(rawrbox::Vector3f{0, 1.F, -1.F}, rawrbox::Colors::Orange() * 0.5F, 5.F);

		// Setup
		this->_model->setOptimizable(false);

		{
			auto mesh = rawrbox::MeshUtils::generateCube({0, 1.0F, 0}, {3.F, 2.F, 0.1F}, rawrbox::Colors::Gray());
			mesh.setRecieveDecals(true);
			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateCube({0, 0.0F, -1.F}, {3.F, 2.F, 0.1F}, rawrbox::Colors::Gray());
			mesh.setRecieveDecals(true);
			mesh.setEulerAngle({bx::toRad(90), 0, 0});

			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateSphere({0.F, 0.F, -1.F}, 0.5F);
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
			this->_model->getMesh(2)->setPos({std::sin(rawrbox::BGFX_FRAME * 0.01F) * 0.5F - 1.F, -0.05F, -0.55F - std::cos(rawrbox::BGFX_FRAME * 0.01F) * 0.5F});
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

		bgfx::dbgTextPrintf(1, 13, 0x1f, "F1 to toggle debug decals");
	}
	void Game::drawWorld() {
		if (!this->_ready) return;
		this->_model->draw();
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
