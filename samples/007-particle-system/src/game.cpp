
#include <rawrbox/render/cameras/orbital.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
#include <rawrbox/render/plugins/particle_engine.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/resources/manager.hpp>

#include <particle_system/game.hpp>

#include <fmt/format.h>

namespace particle_system {
	void Game::setupGLFW() {
#if defined(_DEBUG) && defined(RAWRBOX_SUPPORT_DX12)
		auto* window = rawrbox::Window::createWindow(Diligent::RENDER_DEVICE_TYPE_D3D12); // DX12 is faster on DEBUG than Vulkan, due to vulkan having extra check steps to prevent you from doing bad things
#else
		auto* window = rawrbox::Window::createWindow();
#endif
		window->setMonitor(1);
		window->setTitle("PARTICLE TEST");
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
		render->skipIntros(true);
		render->addPlugin<rawrbox::ParticleEnginePlugin>();
		render->onIntroCompleted = [this]() { this->loadContent(); };
		render->setDrawCall([this](const rawrbox::DrawPass& pass) {
			if (pass != rawrbox::DrawPass::PASS_WORLD) return;
			this->drawWorld();
		});
		// ---------------

		// Setup camera
		auto* cam = render->setupCamera<rawrbox::CameraOrbital>(*window);
		cam->setPos({0.F, 5.F, -5.F});
		cam->setAngle({0.F, rawrbox::MathUtils::toRad(-45), 0.F, 0.F});
		// --------------

		// Add loaders
		rawrbox::RESOURCES::addLoader<rawrbox::TextureLoader>();
		// --------------

		render->init();
	}

	void Game::loadContent() {
		std::vector<std::pair<std::string, uint32_t>> initialContentFiles = {
		    {"./assets/textures/particles.png", 32},
		};

		rawrbox::RESOURCES::loadListAsync(initialContentFiles, [this]() {
			rawrbox::runOnRenderThread([this]() {
				this->contentLoaded();
			});
		});
	}

	void Game::contentLoaded() {
		if (this->_ready) return;

		// Setup model --
		auto mesh = rawrbox::MeshUtils::generateGrid(12, {0.F, 0.F, 0.F});

		this->_model->addMesh(mesh);
		this->_model->upload();
		// --------------

		// Text test ----
		// this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "MESH ID", {0.0F, 0.5F, -0.5F});
		// this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "INSTANCE MESH ID", {0.0F, 0.5F, 0.5F});
		// this->_text->upload();
		// ------

		// Setup emitter ---
		{
			auto* texture = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/particles.png")->get();

			this->_emitter->setTexture(texture);
			this->_emitter->setAtlasIndex(0, 2); // Assuming fire textures are in the first 4 atlas indices

			this->_emitter->setPos({2, 0.1F, 0});                                                                   // Adjust the position as needed
			this->_emitter->setVelocity(rawrbox::Vector3f{-0.5F, 0.F, -0.5F}, rawrbox::Vector3f{0.5F, 0.8F, 0.5F}); // Adjust velocity for fire-like movement

			this->_emitter->setColorTransition({
			    rawrbox::Colorf{1.0F, 1.0F, 0.0F, 1.0F}, // Yellow
			    rawrbox::Colorf{1.0F, 0.5F, 0.0F, 1.0F}, // Orange
			    rawrbox::Colorf{1.0F, 0.0F, 0.0F, 1.0F}, // Red
			    rawrbox::Colorf{0.5F, 0.0F, 0.0F, 0.0F}  // Dark red with fade-out
			});

			this->_emitter->setLifetimeRange(0.5F, 1.5F);                       // Adjust lifetime for fire particles
			this->_emitter->setSpawnRate(10.F);                                 // Increase spawn rate for denser fire
			this->_emitter->setSize(rawrbox::Vector4f{0.2F, 0.2F, 0.2F, 0.2F}); // Adjust size for fire particles

			this->_emitter->setRotation(rawrbox::Vector3f{0.0F, 0.0F, 0.0F}, rawrbox::Vector3f{0.0F, 0.0F, 6.28F}); // Random rotation around Z-axis
			this->_emitter->setGravityMul(0.1F);                                                                    // Adjust gravity for upward movement

			this->_emitter->upload();
		}

		/*{
			auto* texture = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/particles.png")->get();

			this->_emitter2->setTexture(texture);
			this->_emitter2->setAtlasIndex(6, 8); // Assuming fire textures are in the first 4 atlas indices

			this->_emitter2->setPos({-2, 0.1F, 0});                                                                  // Adjust the position as needed
			this->_emitter2->setVelocity(rawrbox::Vector3f{-0.5F, 0.F, -0.5F}, rawrbox::Vector3f{0.5F, 0.8F, 0.5F}); // Adjust velocity for fire-like movement

			this->_emitter2->setColorTransition({
			    rawrbox::Colorf{1.0F, 1.0F, 0.0F, 1.0F}, // Yellow
			    rawrbox::Colorf{1.0F, 0.5F, 0.0F, 1.0F}, // Orange
			    rawrbox::Colorf{1.0F, 0.0F, 0.0F, 1.0F}, // Red
			    rawrbox::Colorf{0.5F, 0.0F, 0.0F, 0.0F}  // Dark red with fade-out
			});

			this->_emitter2->setLifetimeRange(0.5F, 1.5F);                       // Adjust lifetime for fire particles
			this->_emitter2->setSpawnRate(10.F);                                 // Increase spawn rate for denser fire
			this->_emitter2->setSize(rawrbox::Vector4f{0.2F, 0.2F, 0.2F, 0.2F}); // Adjust size for fire particles

			this->_emitter2->setRotation(rawrbox::Vector3f{0.0F, 0.0F, 0.0F}, rawrbox::Vector3f{0.0F, 0.0F, 6.28F}); // Random rotation around Z-axis
			this->_emitter2->setGravityMul(1.0F);                                                                    // Adjust gravity for upward movement

			this->_emitter2->upload();
		}*/
		// -----------------

		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) {
			rawrbox::Window::shutdown();
		} else {
			this->_emitter.reset();
			this->_emitter2.reset();
			this->_model.reset();
			this->_text.reset();
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
		// if (this->_text != nullptr) this->_text->draw();
		if (this->_emitter != nullptr) this->_emitter->draw();
		// if (this->_emitter2 != nullptr) this->_emitter2->draw();
	}

	void Game::draw() {
		rawrbox::Window::render(); // Commit primitives
	}
} // namespace particle_system
