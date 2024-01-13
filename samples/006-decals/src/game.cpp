
#include <rawrbox/engine/static.hpp>
#include <rawrbox/render/cameras/orbital.hpp>
#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
#include <rawrbox/render/plugins/clustered.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/texture.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>

#include <decal_test/game.hpp>

#include <fmt/format.h>

#include <random>

namespace decal_test {
	void Game::setupGLFW() {
		auto window = rawrbox::Window::createWindow();
		window->setMonitor(-1);
		window->setTitle("DECALS TEST");
		window->init(1024, 768, rawrbox::WindowFlags::Window::WINDOWED);
		window->onWindowClose += [this](auto& /*w*/) { this->shutdown(); };
	}

	void Game::init() {
		auto window = rawrbox::Window::getWindow();

		// Setup renderer
		auto render = window->createRenderer();
		render->addPlugin<rawrbox::ClusteredPlugin>();
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

		// BINDS ----
		window->onMouseKey += [](auto&, const rawrbox::Vector2i&, int button, int action, int) {
			const bool isDown = action == 1;
			if (!isDown || button != MOUSE_BUTTON_1) return;
		};
		// -----

		render->init();
	}

	void Game::loadContent() {
		std::array<std::pair<std::string, uint32_t>, 1> initialContentFiles = {
		    std::make_pair<std::string, uint32_t>("./assets/textures/decals.png", 64)};

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
		std::random_device prng;
		std::uniform_int_distribution<uint16_t> dist(0, 4);
		std::uniform_real_distribution<float> distRot(-1.5F, 1.5F);

		auto decalTex = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/decals.png")->get();
		rawrbox::Decal d = {};

		for (int i = 0; i < 30; i++) {
			d.setTexture(*decalTex, dist(prng));
			d.localToWorld = rawrbox::Matrix4x4::mtxSRT({0.5F, 0.5F, 0.5F}, rawrbox::Vector4f::toQuat({rawrbox::MathUtils::toRad(90), 0, 0}), {distRot(prng), 0.F, distRot(prng) - 1.55F});
			d.color = rawrbox::Colors::Green();
			rawrbox::DECALS::add(d);

			d.setTexture(*decalTex, dist(prng));
			d.localToWorld = rawrbox::Matrix4x4::mtxSRT({0.5F, 0.5F, 0.5F}, {}, {distRot(prng), distRot(prng) + 1.25F, 0.F});
			d.color = rawrbox::Colors::Red();
			rawrbox::DECALS::add(d);
		}

		// rawrbox::LIGHTS::addLight<rawrbox::PointLight>(rawrbox::Vector3f{0, 1.F, -1.F}, rawrbox::Colors::White() * 0.5F, 5.F);

		// Setup
		{
			auto mesh = rawrbox::MeshUtils::generateCube({0, 1.0F, 0}, {3.F, 2.F, 0.1F}, rawrbox::Colors::Gray());
			mesh.setRecieveDecals(true);
			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateCube({0, -1.0F, 0.F}, {3.F, 2.F, 0.1F}, rawrbox::Colors::Gray());
			mesh.setRecieveDecals(true);
			mesh.setEulerAngle({rawrbox::MathUtils::toRad(90), 0, 0});

			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateSphere({0.F, 0.F, -1.F}, 0.5F);
			mesh.setOptimizable(false);

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

		if (this->_ready && this->_model != nullptr) {
			this->_model->getMesh(2)->setPos({std::sin(rawrbox::FRAME * 0.01F) * 0.5F - 1.F, -0.05F, -0.55F - std::cos(rawrbox::FRAME * 0.01F) * 0.5F});
		}
	}

	void Game::drawWorld() {
		if (!this->_ready) return;
		this->_model->draw();
	}

	void Game::draw() {
		rawrbox::Window::render(); // Commit primitives
	}
} // namespace decal_test
