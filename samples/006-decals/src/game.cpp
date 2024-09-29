
#include <rawrbox/engine/static.hpp>
#include <rawrbox/render/cameras/orbital.hpp>
#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/lights/point.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
#include <rawrbox/render/plugins/clustered.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>

#include <decal_test/game.hpp>

#include <fmt/format.h>

#include <random>

namespace decal_test {
	void Game::setupGLFW() {
#if defined(_DEBUG) && defined(RAWRBOX_SUPPORT_DX12)
		auto* window = rawrbox::Window::createWindow(Diligent::RENDER_DEVICE_TYPE_D3D12); // DX12 is faster on DEBUG than Vulkan, due to vulkan having extra check steps to prevent you from doing bad things
#else
		auto* window = rawrbox::Window::createWindow();
#endif
		window->setMonitor(-1);
		window->setTitle("DECALS TEST");
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
		render->addPlugin<rawrbox::ClusteredPlugin>();
		render->onIntroCompleted = [this]() { this->loadContent(); };
		render->setDrawCall([this](const rawrbox::DrawPass& pass) {
			if (pass == rawrbox::DrawPass::PASS_WORLD) {
				this->drawWorld();
			} else {
				this->drawOverlay();
			}
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

		// BINDS ----
		window->onMouseKey += [this](auto&, const rawrbox::Vector2i&, int button, int action, int) {
			const bool isDown = action == 1;
			if (!isDown || button != rawrbox::MOUSE_BUTTON_1) return;
			this->generateDecals();
		};
		// -----

		render->init();
	}

	void Game::generateDecals() {
		rawrbox::DECALS::clear();

		std::random_device prng;
		std::uniform_int_distribution<uint16_t> dist(0, 4);
		std::uniform_real_distribution<float> distRot(-0.6F, 0.8F);

		auto* decalTex = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/decals.png")->get();

		rawrbox::Decal d = {};
		for (int i = 0; i < 30; i++) {
			d.setTexture(*decalTex, dist(prng));
			d.setMatrix(rawrbox::Matrix4x4::mtxSRT({0.5F, 0.5F, 0.5F}, rawrbox::Vector4f::toQuat({rawrbox::MathUtils::toRad(90), 0, 0}), {distRot(prng), 0.F, distRot(prng) - 1.2F}));
			d.setColor(rawrbox::Colors::Green());
			rawrbox::DECALS::add(d);

			d.setTexture(*decalTex, dist(prng));
			d.setMatrix(rawrbox::Matrix4x4::mtxSRT({0.5F, 0.5F, 0.5F}, rawrbox::Vector4f::toQuat({0, rawrbox::MathUtils::toRad(180), 0}), {distRot(prng), distRot(prng) + 1.0F, 0.F}));
			d.setColor(rawrbox::Colors::Red());
			rawrbox::DECALS::add(d);
		}
	}

	void Game::loadContent() {
		std::vector<std::pair<std::string, uint32_t>> initialContentFiles = {
		    {"./assets/textures/decals.png", 64},
		};

		rawrbox::RESOURCES::loadListAsync(initialContentFiles, [this]() {
			rawrbox::runOnRenderThread([this]() {
				this->contentLoaded();
			});
		});
	}

	void Game::contentLoaded() {
		if (this->_ready) return;

		// Setup
		{
			auto mesh = rawrbox::MeshUtils::generateCube<rawrbox::MaterialLit>({0, 1.0F, 0}, {3.F, 2.F, 0.1F}, rawrbox::Colors::Gray());
			// mesh.setRecieveDecals(true);
			this->_model2->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateCube<rawrbox::MaterialLit>({0, -1.0F, 0.F}, {3.F, 2.F, 0.1F}, rawrbox::Colors::Gray());
			// mesh.setRecieveDecals(true);
			mesh.setEulerAngle({rawrbox::MathUtils::toRad(90), 0, 0});

			this->_model2->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateSphere({0.F, 0.F, -1.F}, {0.5F, 0.5F, 0.5F});
			mesh.setMergeable(false);

			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateGrid(12, {0.F, 0.F, 0.F});
			this->_model->addMesh(mesh);
		}
		// ----

		this->_model->upload();
		this->_model2->upload();

		this->generateDecals();

		rawrbox::LIGHTS::add<rawrbox::PointLight>(rawrbox::Vector3f{0, 1.F, -1.F}, rawrbox::Colors::White() * 4.5F, 5.F);
		rawrbox::LIGHTS::add<rawrbox::PointLight>(rawrbox::Vector3f{0, 1.F, 1.F}, rawrbox::Colors::White() * 4.5F, 5.F);

		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_RENDER) {
			this->_model.reset();
			this->_model2.reset();

			rawrbox::RESOURCES::shutdown();
		}

		rawrbox::Window::shutdown(thread);
	}

	void Game::pollEvents() {
		rawrbox::Window::pollEvents();
	}

	void Game::update() {
		rawrbox::Window::update();

		if (this->_ready && this->_model != nullptr) {
			this->_model->getMesh(0)->setPos({std::sin(rawrbox::FRAME * 0.01F) * 0.5F - 1.F, -0.05F, -0.55F - std::cos(rawrbox::FRAME * 0.01F) * 0.5F});
		}
	}

	void Game::drawWorld() const {
		if (!this->_ready) return;

		this->_model->draw();
		this->_model2->draw();
	}

	void Game::drawOverlay() const {
		if (!this->_ready) return;
		auto* stencil = rawrbox::RENDERER->stencil();

		stencil->drawText(fmt::format("[MOUSE_1]   RANDOMIZE DECALS"), {15, 15});
	}

	void Game::draw() {
		rawrbox::Window::render(); // Commit primitives
	}
} // namespace decal_test
