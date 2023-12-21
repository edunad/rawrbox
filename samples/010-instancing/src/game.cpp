

#include <rawrbox/engine/static.hpp>
#include <rawrbox/render/cameras/orbital.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/texture.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>
#include <rawrbox/utils/timer.hpp>

#include <instance_test/game.hpp>

#include <fmt/format.h>

#include <random>

namespace instance_test {
	void Game::setupGLFW() {
		auto window = rawrbox::Window::createWindow();
		window->setMonitor(-1);
		window->setTitle("INSTANCE TEST");
		window->init(1024, 768, rawrbox::WindowFlags::Window::WINDOWED);
		window->onWindowClose += [this](auto& /*w*/) { this->shutdown(); };
	}

	void Game::init() {
		auto window = rawrbox::Window::getWindow();

		// Setup renderer
		auto render = window->createRenderer();
		render->onIntroCompleted = [this]() { this->loadContent(); };
		render->setDrawCall([this](const rawrbox::DrawPass& pass) {
			if (pass != rawrbox::DrawPass::PASS_OPAQUE) return;
			this->drawWorld();
		});
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
		    std::make_pair<std::string, uint32_t>("./assets/textures/instance_test.png", 64),
		};

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
		int total = 1000;
		float spacing = 0.85F;

		auto t = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/instance_test.png")->get();
		auto mesh = rawrbox::MeshUtils::generateCube({0, 0, 0}, {0.5F, 0.5F, 0.5F});
		mesh.setTexture(t);

		this->_model->setTemplate(mesh);

		std::random_device prng;
		std::uniform_int_distribution<uint16_t> dist(0, 4);
		std::uniform_real_distribution<float> distRot(0, 360);

		for (int z = 0; z < total; z++) {
			for (int x = 0; x < total; x++) {
				rawrbox::Matrix4x4 m;
				m.SRT({1.F, 1.F, 1.F}, rawrbox::Vector4f::toQuat({0, distRot(prng), 0}), {-500 + x * spacing, dist(prng) * 0.5F, -500 + z * spacing});
				this->_model->addInstance({m, rawrbox::Colors::White(), dist(prng)});
			}
		}

		this->_model->upload();
		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) return;
		this->_model.reset();

		rawrbox::RESOURCES::shutdown();
		rawrbox::ASYNC::shutdown();
		rawrbox::Window::shutdown();
	}

	void Game::pollEvents() {
		rawrbox::Window::pollEvents();
	}

	void Game::update() {
		rawrbox::Window::update();
	}

	void Game::drawWorld() {
		if (!this->_ready || this->_model == nullptr) return;
		this->_model->draw();
	}

	void Game::draw() {
		rawrbox::Window::render(); // Commit primitives
	}
} // namespace instance_test
