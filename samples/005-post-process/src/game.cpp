
#include <rawrbox/render/cameras/orbital.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
#include <rawrbox/render/plugins/post_process.hpp>
#include <rawrbox/render/post_process/dither.hpp>
#include <rawrbox/render/post_process/noise.hpp>
#include <rawrbox/render/post_process/quick_bloom.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>

#include <post_process/game.hpp>

namespace post_process {
	void Game::setupGLFW() {
		auto window = rawrbox::Window::createWindow();
		window->setMonitor(-1);
		window->setTitle("POST-PROCESS TEST");
		window->init(1024, 768, rawrbox::WindowFlags::Window::WINDOWED);
		window->onWindowClose += [this](auto& /*w*/) { this->shutdown(); };
	}

	void Game::init() {
		auto window = rawrbox::Window::getWindow();

		// Setup renderer
		auto render = window->createRenderer();

		// Setup post process ----
		auto postProcess = render->addPlugin<rawrbox::PostProcessPlugin>();
		postProcess->add<rawrbox::PostProcessDither>(rawrbox::DITHER_MODE::FAST_MODE);
		postProcess->add<rawrbox::PostProcessQuickBloom>(0.015F);
		postProcess->add<rawrbox::PostProcessNoise>(0.1F);
		// -----------------------

		render->setDrawCall([this](const rawrbox::DrawPass& pass) {
			if (pass != rawrbox::DrawPass::PASS_OPAQUE) return;
			this->drawWorld();
		});
		render->onIntroCompleted = [this]() {
			this->loadContent();
		};
		// ---------------

		// Setup camera
		auto cam = render->setupCamera<rawrbox::CameraOrbital>(*window);
		cam->setPos({0.F, 5.F, -5.F});
		cam->setAngle({0.F, rawrbox::MathUtils::toRad(-45), 0.F, 0.F});
		// --------------

		// Add loaders
		rawrbox::RESOURCES::addLoader<rawrbox::TextureLoader>();
		// --------------

		render->init();
	}

	void Game::loadContent() {
		std::array<std::pair<std::string, uint32_t>, 1> initialContentFiles = {
		    std::make_pair<std::string, uint32_t>("./assets/textures/crate_hl1.png", 0)};

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
		auto tex = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/crate_hl1.png")->get();

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
			rawrbox::RESOURCES::shutdown();
			rawrbox::ASYNC::shutdown();
		}

		this->_model.reset();
	}

	void Game::pollEvents() {
		rawrbox::Window::pollEvents();
	}

	void Game::update() {
		rawrbox::Window::update();

		if (this->_model != nullptr) {
			this->_model->setEulerAngle({std::cos(rawrbox::FRAME * 0.01F) * 2.5F, std::sin(rawrbox::FRAME * 0.01F) * 2.5F, 0});
		}
	}

	void Game::drawWorld() {
		if (!this->_ready || this->_model == nullptr) return;
		this->_model->draw();
	}

	void Game::draw() {
		rawrbox::Window::render();
	}
} // namespace post_process
