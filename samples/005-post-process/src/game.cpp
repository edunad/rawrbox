
#include <rawrbox/render/cameras/orbital.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
// #include <rawrbox/render/post_process/bloom.hpp>
// #include <rawrbox/render/post_process/dither_psx.hpp>
// #include <rawrbox/render/post_process/static_noise.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>

#include <post_process/game.hpp>

#include "rawrbox/render/post_process/test_1.hpp"
#include "rawrbox/render/post_process/test_2.hpp"

namespace post_process {
	void Game::setupGLFW() {
		auto window = rawrbox::render::createWindow();
		window->setMonitor(-1);
		window->setTitle("POST-PROCESS TEST");
		window->init(1024, 768, rawrbox::WindowFlags::Window::WINDOWED);
		window->onWindowClose += [this](auto& /*w*/) { this->shutdown(); };
	}

	void Game::init() {
		auto window = rawrbox::render::getWindow();

		// Setup renderer
		auto render = rawrbox::render::createRenderer(window);
		render->setOverlayRender([this]() {});
		render->setWorldRender([this]() { this->drawWorld(); });
		render->skipIntros(true);
		render->overridePostWorld([this]() {
			if (!this->_ready) return;
			this->_postProcess->render(rawrbox::render::RENDERER->getColor());
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
		this->_postProcess = std::make_unique<rawrbox::PostProcessManager>();
		this->_postProcess->add<rawrbox::Test1>();
		this->_postProcess->add<rawrbox::Test1>();
		// this->_postProcess->add<rawrbox::Test1>();
		// this->_postProcess->add<rawrbox::Test1>();
		//  this->_postProcess->add<rawrbox::Test1>();

		// this->_postProcess->add<rawrbox::Test2>();

		/*this->_postProcess->add<rawrbox::PostProcessBloom>(0.015F);
		this->_postProcess->add<rawrbox::PostProcessPSXDither>(rawrbox::DITHER_SIZE::SLOW_MODE);
		this->_postProcess->add<rawrbox::PostProcessStaticNoise>(0.1F);*/
		this->_postProcess->upload();

		std::array<std::pair<std::string, uint32_t>, 1> initialContentFiles = {
		    std::make_pair<std::string, uint32_t>("./content/textures/crate_hl1.png", 0)};

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
		auto tex = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("content/textures/crate_hl1.png")->get();

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
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) return;

		this->_model.reset();
		this->_postProcess.reset();

		rawrbox::RESOURCES::shutdown();
		rawrbox::ASYNC::shutdown();
		rawrbox::render::shutdown();
	}

	void Game::pollEvents() {
		rawrbox::render::pollEvents();
	}

	void Game::update() {
		rawrbox::render::update();

		if (this->_model != nullptr) {
			this->_model->setEulerAngle({std::cos(rawrbox::render::FRAME * 0.01F) * 2.5F, std::sin(rawrbox::render::FRAME * 0.01F) * 2.5F, 0});
		}
	}

	void Game::drawWorld() {
		if (!this->_ready || this->_model == nullptr) return;
		this->_model->draw();
	}

	void Game::draw() {
		rawrbox::render::render();
	}
} // namespace post_process
