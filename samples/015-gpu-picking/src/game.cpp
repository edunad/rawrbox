
#include <rawrbox/render/camera/orbital.hpp>
#include <rawrbox/render/model/utils/mesh.hpp>
#include <rawrbox/render/renderers/cluster.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>

#include <gpu/game.hpp>

namespace gpu {

	void Game::setupGLFW() {
		this->_window = std::make_unique<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("GPU PICKING TEST");
		this->_window->setRenderer(
		    bgfx::RendererType::Count, [this]() { this->drawOverlay(); }, [this]() { this->drawWorld(); });
		this->_window->create(1024, 768, rawrbox::WindowFlags::Debug::TEXT | rawrbox::WindowFlags::Debug::PROFILER | rawrbox::WindowFlags::Window::WINDOWED | rawrbox::WindowFlags::Features::MULTI_THREADED);
		this->_window->onWindowClose += [this](auto& /*w*/) { this->shutdown(); };
		this->_window->onIntroCompleted += [this]() {
			this->loadContent();
		};
	}

	void Game::init() {
		if (this->_window == nullptr) return;

		// Setup camera
		auto cam = this->_window->setupCamera<rawrbox::CameraOrbital>(*this->_window);
		cam->setPos({0.F, 5.F, -5.F});
		cam->setAngle({0.F, rawrbox::MathUtils::toRad(-45), 0.F, 0.F});
		// --------------

		// Add loaders ----
		rawrbox::RESOURCES::addLoader<rawrbox::FontLoader>();
		// -----

		this->_window->initializeBGFX();
	}

	void Game::loadContent() {
		std::array<std::pair<std::string, uint32_t>, 1> initialContentFiles = {
		    std::make_pair<std::string, uint32_t>("content/fonts/LiberationMono-Regular.ttf", 0)};

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
		this->_font = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>("content/fonts/LiberationMono-Regular.ttf")->getSize(24);

		// Setup
		{
			auto mesh = rawrbox::MeshUtils::generateGrid(12, {0.F, 0.F, 0.F});
			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateCube({1.0F, 0, 0}, {0.5F, 0.5F, 0.5F});
			mesh.setId(1);

			this->_model2->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateCube({-1.0F, 0, 0}, {0.5F, 0.5F, 0.5F});

			uint32_t ids = 8000;
			for (auto& v : mesh.vertices) {
				v.setId(ids++);
			}

			this->_model->addMesh(mesh);
		}

		// -----
		this->_instance->setTemplate(rawrbox::MeshUtils::generateCube({0, 0, 0}, {0.5F, 0.5F, 0.5F}));

		{
			rawrbox::Matrix4x4 m;
			m.mtxSRT({1.F, 1.F, 1.F}, {}, {-1.F, 0.F, -2.F});
			this->_instance->addInstance({m, rawrbox::Colors::White(), 0, 2});
		}

		{
			rawrbox::Matrix4x4 m;
			m.mtxSRT({1.F, 1.F, 1.F}, {}, {0., 0.F, -2.F});
			this->_instance->addInstance({m, rawrbox::Colors::White(), 0, 3});
		}

		{
			rawrbox::Matrix4x4 m;
			m.mtxSRT({1.F, 1.F, 1.F}, {}, {1., 0.F, -2.F});
			this->_instance->addInstance({m, rawrbox::Colors::White(), 0, 4});
		}
		// ----

		// Text test ----
		this->_text->addText(*this->_font, "MESH ID", {1.0F, 0.5F, 0});
		this->_text->addText(*this->_font, "VERTICE ID", {-1.0F, 0.5F, 0});
		this->_text->addText(*this->_font, "INSTANCE MESH ID", {.0F, 0.5F, -2.0F});
		// ------

		// BINDS ----
		this->_window->onMouseKey += [this](auto& /*w*/, const rawrbox::Vector2i& mousePos, int button, int action, int /*mods*/) {
			const bool isDown = action == 1;
			if (!isDown || button != MOUSE_BUTTON_1) return;

			rawrbox::RENDERER->gpuPick(mousePos, [this](uint32_t id) {
				bool updateModel = false;
				bool updateInstance = false;

				if (this->_lastPicked_vert != nullptr) {
					this->_lastPicked_vert->abgr = rawrbox::Colors::White().pack();
					this->_lastPicked_vert = nullptr;
					updateModel = true;
				}

				if (this->_lastPicked_mesh != nullptr) {
					this->_lastPicked_mesh->setColor(rawrbox::Colors::White());
					this->_lastPicked_mesh = nullptr;
				}

				if (this->_lastPicked_instance != nullptr) {
					this->_lastPicked_instance->setColor(rawrbox::Colors::White());
					this->_lastPicked_instance = nullptr;
					updateInstance = true;
				}

				if (id != 0) {
					for (auto& inst : this->_instance->instances()) {
						if (inst.getId() == id) {
							inst.setColor(rawrbox::Colors::Red());
							this->_lastPicked_instance = &inst;
							updateInstance = true;
							break;
						}
					}

					if (_lastPicked_instance == nullptr) {
						for (auto& mesh : this->_model->meshes()) {
							for (auto& v : mesh->vertices) {
								if (v.id == id) {
									v.abgr = rawrbox::Colors::Red().pack();
									this->_lastPicked_vert = &v;
									updateModel = true;
									break;
								}
							}
						}
					}

					if (this->_lastPicked_vert == nullptr) {
						for (auto& mesh : this->_model2->meshes()) {
							if (mesh->getId() == id) {
								mesh->setColor(rawrbox::Colors::Red());
								this->_lastPicked_mesh = mesh.get();
								break;
							}
						}
					}
				}

				if (updateModel) {
					this->_model->updateBuffers();
				}

				if (updateInstance) {
					this->_instance->updateInstance();
				}
			});
		};
		// -----

		this->_model->upload(true);
		this->_model2->upload();
		this->_instance->upload();
		this->_text->upload();

		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) return;
		this->_model.reset();
		this->_model2.reset();
		this->_instance.reset();
		this->_text.reset();

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

	void Game::drawOverlay() {
		if (!this->_ready) return;

		auto& stencil = this->_window->getStencil();
		auto wSize = this->_window->getSize().cast<float>();

		stencil.drawTexture({wSize.x - 138, 10}, {128, 128}, rawrbox::RENDERER->getGPUPick());
		stencil.render();
	}

	void Game::drawWorld() {
		if (!this->_ready) return;

		this->_model->draw();
		this->_model2->draw();
		this->_instance->draw();
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
		bgfx::dbgTextPrintf(1, 1, 0x1f, "015-gpu-picking");
		bgfx::dbgTextPrintf(1, 2, 0x3f, "Description: GPU Picking test");
		printFrames();
		// -----------

		if (!this->_ready) {
			bgfx::dbgTextPrintf(1, 11, 0x70, "                                   ");
			bgfx::dbgTextPrintf(1, 12, 0x70, "          LOADING CONTENT          ");
			bgfx::dbgTextPrintf(1, 13, 0x70, "                                   ");
		}

		this->_window->render(); // Draw world & commit primitives
	}
} // namespace gpu
