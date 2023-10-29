#include <rawrbox/assimp/importer.hpp>
#include <rawrbox/assimp/resources/model.hpp>
#include <rawrbox/render/cameras/orbital.hpp>
#include <rawrbox/render/light/point.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>
#include <rawrbox/utils/threading.hpp>

#include <assimp/game.hpp>

namespace assimp {

	void Game::setupGLFW() {
		auto window = rawrbox::Window::createWindow();
		window->setMonitor(-1);
		window->setTitle("ASSIMP TEST");
		window->init(1024, 768, rawrbox::WindowFlags::Window::WINDOWED);
		window->onWindowClose += [this](auto& /*w*/) { this->shutdown(); };
	}

	void Game::init() {
		auto window = rawrbox::Window::getWindow();

		// Setup renderer
		auto render = window->createRenderer();
		render->setOverlayRender([this]() {});
		render->setWorldRender([this]() { this->drawWorld(); });
		render->skipIntros(true);
		render->onIntroCompleted = [this]() {
			this->loadContent();
		};
		// ---------------

		// Setup camera
		auto cam = render->setupCamera<rawrbox::CameraOrbital>(*window);
		cam->setPos({0.F, 5.F, -5.F});
		cam->setAngle({0.F, rawrbox::MathUtils::toRad(-45), 0.F, 0.F});
		cam->onMovementStart = []() { fmt::print("Camera start\n"); };
		cam->onMovementStop = []() { fmt::print("Camera stop\n"); };
		// --------------

		// Add loaders
		rawrbox::RESOURCES::addLoader<rawrbox::AssimpLoader>();
		rawrbox::RESOURCES::addLoader<rawrbox::FontLoader>();
		// --------------

		render->init();
	}

	void Game::loadContent() {
		std::array initialContentFiles = {
		    std::make_pair<std::string, uint32_t>("./assets/fonts/LiberationMono-Regular.ttf", 0),
		    std::make_pair<std::string, uint32_t>("./assets/models/shape_keys/shape_keys.glb", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_BLEND_SHAPES | rawrbox::ModelLoadFlags::Debug::PRINT_BLENDSHAPES),
		    std::make_pair<std::string, uint32_t>("./assets/models/ps1_phasmophobia/Phasmaphobia_Semi.fbx", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_LIGHT),
		    std::make_pair<std::string, uint32_t>("./assets/models/wolf/wolfman_animated.fbx", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS | rawrbox::ModelLoadFlags::Debug::PRINT_METADATA),
		    std::make_pair<std::string, uint32_t>("./assets/models/multiple_skeleton/twocubestest.gltf", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS | rawrbox::ModelLoadFlags::Debug::PRINT_BONE_STRUCTURE),
		    std::make_pair<std::string, uint32_t>("./assets/models/grandma_tv/scene.gltf", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS | rawrbox::ModelLoadFlags::Debug::PRINT_MATERIALS)};

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
		this->_font = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>("./assets/fonts/LiberationMono-Regular.ttf")->getSize(24);

		// Assimp test ---
		auto mdl = rawrbox::RESOURCES::getFile<rawrbox::ResourceAssimp>("./assets/models/ps1_phasmophobia/Phasmaphobia_Semi.fbx")->get();

		this->_model->setPos({7, 1.1F, 0.F});
		this->_model->load(*mdl);
		// this->_model->upload();

		// this->_model2->setMaterial<rawrbox::MaterialLit>();
		this->_model2->setPos({-6, 1.1F, 0.F});
		this->_model2->load(*mdl);
		// this->_model2->upload();

		// ANIMATIONS ---
		auto mdl2 = rawrbox::RESOURCES::getFile<rawrbox::ResourceAssimp>("./assets/models/wolf/wolfman_animated.fbx")->get();
		this->_model3->load(*mdl2);
		this->_model3->playAnimation("Scene", true, 1.F);
		this->_model3->setPos({0, 0, 0});
		// this->_model3->upload();

		auto mdl3 = rawrbox::RESOURCES::getFile<rawrbox::ResourceAssimp>("./assets/models/multiple_skeleton/twocubestest.gltf")->get();
		this->_model4->load(*mdl3);
		this->_model4->playAnimation("MewAction", true, 0.8F);
		this->_model4->playAnimation("MewAction.001", true, 0.5F);
		this->_model4->setPos({0, 0, 2.5F});
		this->_model4->setScale({0.25F, 0.25F, 0.25F});
		this->_model4->upload();

		auto mdl4 = rawrbox::RESOURCES::getFile<rawrbox::ResourceAssimp>("./assets/models/grandma_tv/scene.gltf")->get();
		this->_model5->load(*mdl4);
		// this->_model5->playAnimation("Scene", true, 1.F);
		this->_model5->setPos({0, 0, -3.5F});
		this->_model5->setScale({0.35F, 0.35F, 0.35F});
		this->_model5->setEulerAngle({0, rawrbox::MathUtils::toRad(90.F), 0});
		// this->_model5->upload();

		auto mdl5 = rawrbox::RESOURCES::getFile<rawrbox::ResourceAssimp>("./assets/models/shape_keys/shape_keys.glb")->get();
		this->_model6->load(*mdl5);
		this->_model6->setScale({0.4F, 0.4F, 0.4F});
		this->_model6->setPos({2.F, 0.4F, -6.F});
		// this->_model6->upload(true);
		//    -----

		// Text test ----
		{
			this->_text->addText(*this->_font, "TEXTURES + LIGHT", {-6.F, 3.0F, 0});
			this->_text->addText(*this->_font, "TEXTURES", {6.F, 3.0F, 0});
			this->_text->addText(*this->_font, "SINGLE ARMATURE +\nVERTEX ANIMATION", {0.F, 2.F, 0});
			this->_text->addText(*this->_font, "TWO ARMATURES +\nTWO ANIMATIONS", {0.F, 1.F, 2.5F});
			this->_text->addText(*this->_font, "VERTEX ANIMATIONS", {0.F, 1.8F, -3.5F});
			this->_text->addText(*this->_font, "EMBEDDED TEXTURES +\nBLEND SHAPES", {2.F, 1.8F, -6.F});
			// this->_text->upload();
		}
		// ------

		{
			auto mesh = rawrbox::MeshUtils::generateGrid(24, {0.F, 0.F, 0.F});
			this->_modelGrid->addMesh(mesh);
			// this->_modelGrid->upload();
		}

		// LIGHT ----
		rawrbox::LIGHTS::addLight<rawrbox::PointLight>(rawrbox::Vector3f{2.F, 1.8F, -6.F}, rawrbox::Colors::White(), 6.2F);
		// -----------

		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) return;

		this->_model.reset();
		this->_model2.reset();
		this->_model3.reset();
		this->_model4.reset();
		this->_model5.reset();
		this->_model6.reset();
		this->_modelGrid.reset();

		this->_text.reset();

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
		if (!this->_ready) return;
		// this->_modelGrid->draw();

		// this->_model->draw();
		// this->_model2->draw();
		// this->_model3->draw();
		this->_model4->draw();
		// this->_model5->draw();
		//
		//  this->_model6->setBlendShape("Cheese-Melt", std::abs(std::cos(rawrbox::FRAME * 0.005F) * 1.F));
		//  this->_model6->setBlendShape("Other-Nya", std::abs(std::cos(rawrbox::FRAME * 0.008F) * 1.F));
		//  this->_model6->setBlendShape("Other-Melt", std::abs(std::cos(rawrbox::FRAME * 0.002F) * 1.F));
		//  this->_model6->draw();

		// this->_text->draw();
	}

	void Game::draw() {
		rawrbox::Window::render(); // Commit primitives
	}
} // namespace assimp
