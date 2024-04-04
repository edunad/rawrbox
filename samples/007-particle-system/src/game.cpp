
#include <rawrbox/render/cameras/orbital.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
#include <rawrbox/render/plugins/particle_engine.hpp>
#include <rawrbox/render/static.hpp>

#include <particle_system/game.hpp>

#include <fmt/format.h>

namespace particle_system {
	void Game::setupGLFW() {
#if defined(_DEBUG) && defined(RAWRBOX_SUPPORT_DX12)
		auto* window = rawrbox::Window::createWindow(Diligent::RENDER_DEVICE_TYPE_D3D12); // DX12 is faster on DEBUG than Vulkan, due to vulkan having extra check steps to prevent you from doing bad things
#else
		auto* window = rawrbox::Window::createWindow();
#endif
		window->setMonitor(-1);
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

		render->init();
	}

	void Game::loadContent() {
		this->contentLoaded(); // NO CONTENT
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
		this->_emitter->setPos({0, 1.F, 0});
		this->_emitter->setVelocity(rawrbox::Vector3f{-1.F, 0, -1}, rawrbox::Vector3f{1.0F, 1.0F, 1.0F});

		this->_emitter->setColorTransition({rawrbox::Colorf{1.0F, 1.0F, 1.0F, 1.0F},
		    rawrbox::Colorf{1.0F, 1.0F, 1.0F, 1.0F},
		    rawrbox::Colorf{1.0F, 1.0F, 1.0F, 1.0F},
		    rawrbox::Colorf{1.0F, 1.0F, 1.0F, 0.0F}});

		this->_emitter->setLifetimeRange(1.0F, 5.0F);
		this->_emitter->setSpawnRate(5.F);
		this->_emitter->setSize(rawrbox::Vector4f{0.1F, 0.1F, 0.05F, 0.05F});

		this->_emitter->upload();
		// -----------------

		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) {
			rawrbox::Window::shutdown();
		} else {
			this->_emitter.reset();
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
	}

	void Game::draw() {
		rawrbox::Window::render(); // Commit primitives
	}
} // namespace particle_system
