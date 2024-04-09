
#include <rawrbox/render/cameras/orbital.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
#include <rawrbox/render/plugins/post_process.hpp>
#include <rawrbox/render/post_process/dither.hpp>
#include <rawrbox/render/post_process/fog.hpp>
#include <rawrbox/render/post_process/noise.hpp>
#include <rawrbox/render/post_process/quick_bloom.hpp>
#include <rawrbox/render/post_process/skybox.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>

#include <post_process/game.hpp>

namespace post_process {
	void Game::setupGLFW() {
#if defined(_DEBUG) && defined(RAWRBOX_SUPPORT_DX12)
		auto* window = rawrbox::Window::createWindow(Diligent::RENDER_DEVICE_TYPE_D3D12); // DX12 is faster on DEBUG than Vulkan, due to vulkan having extra check steps to prevent you from doing bad things
#else
		auto* window = rawrbox::Window::createWindow();
#endif
		window->setMonitor(-1);
		window->setTitle("POST-PROCESS TEST");
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

		// Setup post process ----
		auto* postProcess = render->addPlugin<rawrbox::PostProcessPlugin>();
		this->_skybox = postProcess->add<rawrbox::PostProcessSkybox>(rawrbox::Colors::Blue(), rawrbox::Colors::White());

		this->_fog = postProcess->add<rawrbox::PostProcessFog>();
		this->_fog->setEnabled(false);

		this->_dither = postProcess->add<rawrbox::PostProcessDither>(rawrbox::DITHER_MODE::SLOW_MODE);
		this->_bloom = postProcess->add<rawrbox::PostProcessQuickBloom>(0.015F);
		this->_noise = postProcess->add<rawrbox::PostProcessNoise>(0.1F);
		//   -----------------------

		render->setDrawCall([this](const rawrbox::DrawPass& pass) {
			if (pass == rawrbox::DrawPass::PASS_WORLD) {
				this->drawWorld();
			} else {
				this->drawOverlay();
			}
		});
		render->onIntroCompleted = [this]() {
			this->loadContent();
		};
		// ---------------

		// Setup camera
		auto* cam = render->setupCamera<rawrbox::CameraOrbital>(*window);
		cam->setPos({0.F, 5.F, -5.F});
		cam->setAngle({0.F, rawrbox::MathUtils::toRad(-45), 0.F, 0.F});
		// --------------

		// BINDS ----
		window->onKey += [this](rawrbox::Window& /*w*/, uint32_t key, uint32_t /*scancode*/, uint32_t action, uint32_t /*mods*/) {
			if (!this->_ready || action != rawrbox::KEY_ACTION_UP) return;

			switch (key) {
				case rawrbox::KEY_F1:
					this->_skybox->setEnabled(!this->_skybox->isEnabled());
					break;
				case rawrbox::KEY_F2:
					this->_dither->setEnabled(!this->_dither->isEnabled());
					break;
				case rawrbox::KEY_F3:
					this->_fog->setEnabled(!this->_fog->isEnabled());
					break;
				case rawrbox::KEY_F4:
					this->_bloom->setEnabled(!this->_bloom->isEnabled());
					break;
				case rawrbox::KEY_F5:
					this->_noise->setEnabled(!this->_noise->isEnabled());
					break;
				default: break;
			}
		};
		// -----

		// Add loaders
		rawrbox::RESOURCES::addLoader<rawrbox::TextureLoader>();
		// --------------

		render->init();
	}

	void Game::loadContent() {
		std::vector<std::pair<std::string, uint32_t>> initialContentFiles = {
		    {"./assets/textures/crate_hl1.png", 0},
		};

		rawrbox::RESOURCES::loadListAsync(initialContentFiles, [this]() {
			rawrbox::runOnRenderThread([this]() {
				this->contentLoaded();
			});
		});
	}

	void Game::contentLoaded() {
		if (this->_ready) return;
		auto* tex = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/crate_hl1.png")->get();

		// Setup
		{
			auto mesh = rawrbox::MeshUtils::generateCube({0.F, 0.F, 0}, {3.F, 3.F, 3.F}, rawrbox::Colors::White());
			mesh.setTexture(tex);
			this->_model->addMesh(mesh);
		}
		// ----

		this->_model->upload();
		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) {
			rawrbox::Window::shutdown();
		} else {
			this->_model.reset();

			rawrbox::RESOURCES::shutdown();
		}
	}

	void Game::pollEvents() {
		rawrbox::Window::pollEvents();
	}

	void Game::update() {
		rawrbox::Window::update();

		if (this->_model != nullptr) {
			this->_model->setEulerAngle({std::cos(rawrbox::FRAME * 0.001F) * 2.5F, std::sin(rawrbox::FRAME * 0.001F) * 2.5F, 0});
		}
	}

	void Game::drawWorld() {
		if (!this->_ready || this->_model == nullptr) return;
		this->_model->draw();
	}

	void Game::drawOverlay() const {
		if (!this->_ready) return;
		auto* stencil = rawrbox::RENDERER->stencil();

		stencil->drawText(fmt::format("[F1]   SKYBOX -> {}", this->_skybox->isEnabled() ? "enabled" : "disabled"), {15, 15}, rawrbox::Colors::White(), rawrbox::Colors::Black());
		stencil->drawText(fmt::format("[F2]   DITHER -> {}", this->_dither->isEnabled() ? "enabled" : "disabled"), {15, 30}, rawrbox::Colors::White(), rawrbox::Colors::Black());
		stencil->drawText(fmt::format("[F3]   FOG -> {}", this->_fog->isEnabled() ? "enabled" : "disabled"), {15, 45}, rawrbox::Colors::White(), rawrbox::Colors::Black());
		stencil->drawText(fmt::format("[F4]   BLOOM -> {}", this->_bloom->isEnabled() ? "enabled" : "disabled"), {15, 60}, rawrbox::Colors::White(), rawrbox::Colors::Black());
		stencil->drawText(fmt::format("[F5]   NOISE -> {}", this->_noise->isEnabled() ? "enabled" : "disabled"), {15, 75}, rawrbox::Colors::White(), rawrbox::Colors::Black());
	}

	void Game::draw() {
		rawrbox::Window::render();
	}
} // namespace post_process
