
#include <rawrbox/render/cameras/orbital.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
#include <rawrbox/render/static.hpp>

#include <gpu/game.hpp>

#include <fmt/format.h>

namespace gpu {
	void Game::setupGLFW() {
#if defined(_DEBUG) && defined(RAWRBOX_SUPPORT_DX12)
		auto* window = rawrbox::Window::createWindow(Diligent::RENDER_DEVICE_TYPE_D3D12); // DX12 is faster on DEBUG than Vulkan, due to vulkan having extra check steps to prevent you from doing bad things
#else
		auto window = rawrbox::Window::createWindow();
#endif
		window->setMonitor(-1);
		window->setTitle("GPU PICKING TEST");
#ifdef _DEBUG
		window->init(1024, 768, rawrbox::WindowFlags::Window::WINDOWED);
#else
		window->init(-1, -1, rawrbox::WindowFlags::Window::BORDERLESS);
#endif

		window->onWindowClose += [this](auto& /*w*/) { this->shutdown(); };
	}

	void Game::init() {
		auto* window = rawrbox::Window::getWindow();

		// Setup renderer
		auto* render = window->createRenderer();
		render->skipIntros(true);
		render->onIntroCompleted = [this]() { this->loadContent(); };
		render->setDrawCall([this](const rawrbox::DrawPass& pass) {
			if (pass != rawrbox::DrawPass::PASS_OPAQUE) return;
			this->drawWorld();
		});
		// ---------------

		// Setup camera
		auto* cam = render->setupCamera<rawrbox::CameraOrbital>(*window);
		cam->setPos({0.F, 5.F, -5.F});
		cam->setAngle({0.F, rawrbox::MathUtils::toRad(-45), 0.F, 0.F});
		// --------------

		render->init();

		// BINDS ----
		window->onMouseKey += [this](auto& /*w*/, const rawrbox::Vector2i& mousePos, int button, int action, int /*mods*/) {
			const bool isDown = action == 1;
			if (!this->_ready || !isDown || button != MOUSE_BUTTON_1) return;

			rawrbox::RENDERER->gpuPick(mousePos, [this](uint32_t id) {
				bool updateInstance = false;

				if (this->_lastPickedMesh != nullptr) {
					this->_lastPickedMesh->setColor(rawrbox::Colors::White());
					this->_lastPickedMesh = nullptr;
				}

				if (this->_lastPickedInstance != nullptr) {
					this->_lastPickedInstance->setColor(rawrbox::Colors::White());
					this->_lastPickedInstance = nullptr;

					updateInstance = true;
				}

				if (id != 0) {
					// Check model -----
					for (auto& mesh : this->_model->meshes()) {
						if (mesh->getID() == id) {
							mesh->setColor(rawrbox::Colors::Red());
							this->_lastPickedMesh = mesh.get();
							break;
						}
					}
					// -----------------

					// Check instance --
					for (auto& inst : this->_instance->instances()) {
						if (inst.getId() == id) {
							inst.setColor(rawrbox::Colors::Red());
							this->_lastPickedInstance = &inst;
							updateInstance = true;
							break;
						}
					}
					// -----------------
				}

				if (updateInstance) this->_instance->updateInstance();
			});
		};
		// -----
	}

	void Game::loadContent() {
		this->contentLoaded(); // NO CONTENT
	}

	void Game::contentLoaded() {
		if (this->_ready) return;

		// Normal
		{
			auto mesh = rawrbox::MeshUtils::generateGrid(12, {0.F, 0.F, 0.F});
			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateCube({-0.5F, 0, -0.5F}, {0.5F, 0.5F, 0.5F});
			mesh.setID(1);

			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateCube({0.5F, 0, -0.5F}, {0.5F, 0.5F, 0.5F});
			mesh.setID(2);

			this->_model->addMesh(mesh);
		}
		// ----

		// Instanced --
		this->_instance->setTemplate(rawrbox::MeshUtils::generateCube({0, 0, 0}, {0.5F, 0.5F, 0.5F}));

		{
			rawrbox::Matrix4x4 m = rawrbox::Matrix4x4::mtxSRT({1.F, 1.F, 1.F}, {}, {-0.5F, 0, 0.5F});
			this->_instance->addInstance({m, rawrbox::Colors::White(), 0, 3});
		}

		{
			rawrbox::Matrix4x4 m = rawrbox::Matrix4x4::mtxSRT({1.F, 1.F, 1.F}, {}, {0.5F, 0, 0.5F});
			this->_instance->addInstance({m, rawrbox::Colors::White(), 0, 4});
		}
		// ------------

		// Text test ----
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "MESH ID", {0.0F, 0.5F, -0.5F});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "INSTANCE MESH ID", {0.0F, 0.5F, 0.5F});
		// ------

		this->_model->upload();
		this->_instance->upload();
		this->_text->upload();
		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) {
			rawrbox::Window::shutdown();
		} else {
			this->_model.reset();
			this->_instance.reset();
			this->_text.reset();

			this->_lastPickedMesh = nullptr;
		}
	}

	void Game::pollEvents() {
		rawrbox::Window::pollEvents();
	}

	void Game::update() {
		rawrbox::Window::update();
	}

	void Game::drawWorld() {
		if (!this->_ready) return;

		if (this->_model != nullptr) this->_model->draw();
		if (this->_instance != nullptr) this->_instance->draw();
		if (this->_text != nullptr) this->_text->draw();
	}

	void Game::draw() {
		rawrbox::Window::render(); // Commit primitives
	}
} // namespace gpu
