
#include <rawrbox/render/cameras/orbital.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/resources/manager.hpp>

#include <camera/game.hpp>

#include <fmt/format.h>

namespace camera {
	void Game::setupGLFW() {
#if defined(_DEBUG) && defined(RAWRBOX_SUPPORT_DX12)
		auto* window = rawrbox::Window::createWindow(Diligent::RENDER_DEVICE_TYPE_D3D12); // DX12 is faster on DEBUG than Vulkan, due to vulkan having extra check steps to prevent you from doing bad things
#else
		auto* window = rawrbox::Window::createWindow();
#endif
		window->setMonitor(1);
		window->setTitle("CAMERA TEST");
#ifdef _DEBUG
		window->init(1600, 900, rawrbox::WindowFlags::Window::WINDOWED);
#else
		window->init(0, 0, rawrbox::WindowFlags::Window::BORDERLESS);
#endif

		window->onWindowClose += [this](auto& /*w*/) { this->shutdown(); };
	}

	void Game::init() {
		auto* window = rawrbox::Window::getWindow();

		// Setup renderer
		auto* render = window->createRenderer();
		render->onIntroCompleted = [this]() { this->loadContent(); };
		render->setDrawCall([this](const rawrbox::CameraBase& camera, const rawrbox::DrawPass& pass) {
			if (pass == rawrbox::DrawPass::PASS_OVERLAY) {
				this->drawOverlay();
			} else {
				this->drawWorld(camera.getLayers());
			}
		});
		// ---------------

		// Setup main camera
		auto* cam = render->createCamera<rawrbox::CameraOrbital>(*window);
		cam->setPos({0.F, 6.F, -6.F});
		cam->setAngle({0.F, rawrbox::MathUtils::toRad(-55), 0.F, 0.F});
		// --------------

		// Setup camera 2
		this->_camera = render->createCamera<rawrbox::CameraPerspective>(rawrbox::Vector2u{256, 256});
		this->_camera->setPos({0.F, 6.F, -6.F});
		this->_camera->setAngle({0.F, rawrbox::MathUtils::toRad(-55), 0.F, 0.F});
		this->_camera->setLayers(rawrbox::CameraLayers::LAYER_1); // Only draw objects on layer 1 (the name can be changed on the render_config.hpp file)
		// --------------

		// Add loaders
		rawrbox::RESOURCES::addLoader<rawrbox::TextureLoader>();
		// --------------

		// BINDS ----
		window->onKey += [this](rawrbox::Window& /*w*/, uint32_t key, uint32_t /*scancode*/, uint32_t action, uint32_t /*mods*/) {
			if (!this->_ready || action != rawrbox::KEY_ACTION_UP || key != rawrbox::KEY_F1) return;
			this->_layers = !this->_layers;
		};
		// -----

		render->init();
	}

	void Game::loadContent() {
		std::vector<std::pair<std::string, uint32_t>> initialContentFiles = {
		    {"./assets/textures/fire1.gif", 0},
		    {"./assets/textures/screem.png", 0},
		};

		rawrbox::RESOURCES::loadListAsync(initialContentFiles, [this]() {
			rawrbox::runOnRenderThread([this]() {
				this->contentLoaded();
			});
		});
	}

	void Game::contentLoaded() {
		if (this->_ready) return;

		auto* texture = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/fire1.gif")->get();
		auto* texture2 = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/screem.png")->get();

		// SETUP MODELS ---
		{
			auto mesh = rawrbox::MeshUtils::generateCube({1.5F, 0, 0}, {1.0F, 1.0F, 1.0F}, rawrbox::Colors::White());
			mesh.setTexture(texture);
			this->_model->addMesh(mesh);
			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({1.5F, 0, 0}, mesh.getBBOX()));
		}

		{
			auto mesh = rawrbox::MeshUtils::generatePlane({0, 0, 0}, {1.0F, 1.0F}, rawrbox::Colors::White());
			mesh.setTexture(this->_camera->getRenderTarget());
			this->_model->addMesh(mesh);
			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({0, 0, 0}, mesh.getBBOX()));
		}

		{
			auto mesh = rawrbox::MeshUtils::generateCube({-1.5F, 0, 0}, {1.0F, 1.0F, 1.0F}, rawrbox::Colors::White());
			mesh.setTexture(texture2);
			this->_model2->addMesh(mesh);
			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({-1.5F, 0, 0}, mesh.getBBOX()));
		}
		// ----------------

		// TEXT ---
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "ONLY VISIBLE IN CAMERA 2", {-1.5F, 1.2F, 0});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "CAMERA 2 RENDER TARGET", {0, 0.6F, 0});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "VISIBLE ON BOTH CAMERAS", {1.5F, 1.2F, 0});
		this->_text->upload();
		// --------

		this->_model->upload();
		this->_model2->upload();
		this->_bboxes->upload();

		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_RENDER) {
			this->_model.reset();
			this->_model2.reset();
			this->_bboxes.reset();
			this->_text.reset();

			this->_camera = nullptr;

			rawrbox::RESOURCES::shutdown();
		}

		rawrbox::Window::shutdown(thread);
	}

	void Game::pollEvents() {
		rawrbox::Window::pollEvents();
	}

	void Game::update() {
		rawrbox::Window::update();
	}

	void Game::drawWorld(uint32_t layer) {
		if (!this->_ready) return;

		this->_model->draw();

		if (this->_layers) {
			if ((layer & rawrbox::CameraLayers::LAYER_1) > 0) {
				this->_model2->draw();
			}
		} else {
			this->_model2->draw();
		}

		this->_bboxes->draw();
		this->_text->draw();
	}

	void Game::drawOverlay() {
		if (!this->_ready) return;
		auto* stencil = rawrbox::RENDERER->stencil();

		stencil->drawText(fmt::format("[F1]   LAYERS -> {}", this->_layers ? "enabled" : "disabled"), {15, 15}, rawrbox::Colors::White(), rawrbox::Colors::Black());
	}

	void Game::draw() {
		rawrbox::Window::render(); // Commit primitives
	}
} // namespace camera
