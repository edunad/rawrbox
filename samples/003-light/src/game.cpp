
#include <rawrbox/render/cameras/orbital.hpp>
#include <rawrbox/render/lights/directional.hpp>
#include <rawrbox/render/lights/point.hpp>
#include <rawrbox/render/lights/spot.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
#include <rawrbox/render/plugins/clustered.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>

#include <light/game.hpp>

namespace light {

	void Game::setupGLFW() {
#ifdef _DEBUG
		auto* window = rawrbox::Window::createWindow(Diligent::RENDER_DEVICE_TYPE_D3D12); // DX12 is faster on DEBUG than Vulkan, due to vulkan having extra check steps to prevent you from doing bad things
#else
		auto window = rawrbox::Window::createWindow();
#endif
		window->setMonitor(-1);
		window->setTitle("LIGHT TEST");
		window->init(1024, 768, rawrbox::WindowFlags::Window::WINDOWED);
		window->onWindowClose += [this](auto& /*w*/) { this->shutdown(); };
	}

	void Game::init() {
		auto* window = rawrbox::Window::getWindow();

		// Setup renderer
		auto* render = window->createRenderer();
		render->addPlugin<rawrbox::ClusteredPlugin>();
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

		// Add loaders ----
		rawrbox::RESOURCES::addLoader<rawrbox::FontLoader>();
		rawrbox::RESOURCES::addLoader<rawrbox::TextureLoader>();
		// -----

		render->init();
	}

	void Game::loadContent() {
		std::vector<std::pair<std::string, uint32_t>> initialContentFiles = {
		    {"./assets/textures/decals.png", 64},
		    {"./assets/textures/light_test/planks.png", 0},
		    {"./assets/textures/light_test/planksSpec.png", 0},
		    {"./assets/textures/light_test/planksNorm.png", 0}};

		rawrbox::RESOURCES::loadListAsync(initialContentFiles, [this]() {
			rawrbox::runOnRenderThread([this]() {
				rawrbox::BindlessManager::processBarriers(); // IMPORTANT: BARRIERS NEED TO BE PROCESSED AFTER LOADING ALL THE CONTENT
				this->contentLoaded();
			});
		});
	}

	void Game::contentLoaded() {
		if (this->_ready) return;

		auto* tex = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/light_test/planks.png")->get();
		auto* texNorm = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/light_test/planksNorm.png")->get();

		// Setup
		{
			auto mesh = rawrbox::MeshUtils::generateGrid<>(12, {0.F, 0.F, 0.F});
			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generatePlane<rawrbox::MaterialLit>({3.5F, 0.F, -0.01F}, {3.F, 3.F}, rawrbox::Colors::White());

			mesh.setEulerAngle({rawrbox::MathUtils::toRad(90), 0, 0});
			mesh.setTexture(tex);
			mesh.setNormalTexture(texNorm);

			this->_model2->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generatePlane<rawrbox::MaterialLit>({-3.5F, 0.F, -0.01F}, {3.F, 3.F}, rawrbox::Colors::White());

			mesh.setEulerAngle({rawrbox::MathUtils::toRad(90), 0, 0});
			mesh.setTexture(tex);
			mesh.setNormalTexture(texNorm);

			this->_model2->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generatePlane<rawrbox::MaterialInstancedLit>({0, 0, 0}, {0.25F, 0.25F});

			mesh.setEulerAngle({rawrbox::MathUtils::toRad(90), 0, 0});
			mesh.setTexture(tex);
			mesh.setNormalTexture(texNorm);

			this->_model3->setTemplate(mesh);
		}

		for (size_t y = 0; y < 3; y++) {
			for (size_t x = 0; x < 3; x++) {
				rawrbox::Matrix4x4 m;
				m.SRT({1.F, 1.F, 1.F}, rawrbox::Vector4f::toQuat({0, 0, 0}), {x * 0.5F - 0.5F, y * 0.5F - 0.5F, -0.01F});

				this->_model3->addInstance({m, rawrbox::Colors::White(), 0});
			}
		}
		// ----

		// Text test ----
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "POINT LIGHT", {-3.5F, 0.5F, 0});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "SPOT LIGHT", {3.5F, 0.5F, 0});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "INSTANCES", {0.F, 0.5F, 0});
		// ------

		// Light test ----------
		rawrbox::LIGHTS::add<rawrbox::PointLight>(rawrbox::Vector3f{-3.5F, 0.2F, 0}, rawrbox::Colors::Blue() * 50, 1.2F);
		rawrbox::LIGHTS::add<rawrbox::SpotLight>(rawrbox::Vector3f{3.5F, 1.F, 0}, rawrbox::Vector3f{0.F, -1.F, 0.F}, rawrbox::Colors::Purple() * 50, 20.F, 40.F, 4.F);
		rawrbox::LIGHTS::add<rawrbox::PointLight>(rawrbox::Vector3f{0.2F, 0.2F, 0}, rawrbox::Colors::Orange() * 50, 1.F);

		rawrbox::LIGHTS::add<rawrbox::DirectionalLight>(rawrbox::Vector3f{0.F, 10.F, 0}, rawrbox::Vector3f{0.F, -1.F, 0.F}, rawrbox::Colors::White()); // SUN
		// -------------------

		// Decal test --------
		auto* decalTex = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/decals.png")->get();

		rawrbox::Decal d = {};
		d.setTexture(*decalTex, 3);
		d.localToWorld = rawrbox::Matrix4x4::mtxSRT({0.5F, 0.5F, 0.5F}, rawrbox::Vector4f::toQuat({rawrbox::MathUtils::toRad(90), 0, 0}), {-3.5F, 0, 0});

		rawrbox::DECALS::add(d);
		// -------------------

		this->_model->upload();
		this->_model2->upload();
		this->_model3->upload();

		this->_text->upload();

		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) {
			rawrbox::Window::shutdown();
		} else {
			this->_model.reset();
			this->_model2.reset();
			this->_model3.reset();
			this->_text.reset();

			rawrbox::RESOURCES::shutdown();
		}
	}

	void Game::pollEvents() {
		rawrbox::Window::pollEvents();
	}

	void Game::update() {
		rawrbox::Window::update();

		if (this->_ready) {
			auto* light = rawrbox::LIGHTS::getLight(0);
			if (light != nullptr) {
				light->setOffsetPos({std::sin(rawrbox::FRAME * 0.01F) * 0.5F, 0, std::cos(rawrbox::FRAME * 0.01F) * 0.5F});
			}

			light = rawrbox::LIGHTS::getLight(1);
			if (light != nullptr) {
				light->setOffsetPos({0, std::cos(rawrbox::FRAME * 0.01F) * 1.F, 0});
			}

			light = rawrbox::LIGHTS::getLight(2);
			if (light != nullptr) {
				light->setOffsetPos({std::sin(rawrbox::FRAME * 0.01F) * 0.5F, 0, std::cos(rawrbox::FRAME * 0.01F) * 0.5F});
			}

			light = rawrbox::LIGHTS::getLight(3); // SUN
			if (light != nullptr) {
				light->setDirection({0, std::sin(rawrbox::FRAME * 0.01F) * 1.F, 0});
			}
		}
	}

	void Game::drawWorld() {
		if (!this->_ready || this->_model == nullptr || this->_model2 == nullptr || this->_text == nullptr) return;

		this->_model->draw();
		this->_model2->draw();
		this->_model3->draw();

		this->_text->draw();
	}

	void Game::draw() {
		rawrbox::Window::render(); // Commit primitives
	}
} // namespace light
