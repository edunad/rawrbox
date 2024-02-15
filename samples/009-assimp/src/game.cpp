#include <rawrbox/assimp/importer.hpp>
#include <rawrbox/assimp/resources/model.hpp>
#include <rawrbox/render/cameras/orbital.hpp>
#include <rawrbox/render/lights/point.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
#include <rawrbox/render/plugins/clustered.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>
#include <rawrbox/utils/threading.hpp>

#include <assimp/game.hpp>

namespace assimp {

	void Game::setupGLFW() {
#ifdef _DEBUG
		auto window = rawrbox::Window::createWindow(Diligent::RENDER_DEVICE_TYPE_D3D12); // DX12 is faster on DEBUG than Vulkan, due to vulkan having extra check steps to prevent you from doing bad things
#else
		auto window = rawrbox::Window::createWindow();
#endif

		window->setMonitor(-1);
		window->setTitle("ASSIMP TEST");
		window->init(1024, 768, rawrbox::WindowFlags::Window::WINDOWED);
		window->onWindowClose += [this](auto& /*w*/) { this->shutdown(); };
	}

	void Game::init() {
		auto window = rawrbox::Window::getWindow();

		// Setup renderer
		auto render = window->createRenderer();
		render->onIntroCompleted = [this]() { this->loadContent(); };
		render->addPlugin<rawrbox::ClusteredPlugin>();
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
		rawrbox::RESOURCES::addLoader<rawrbox::AssimpLoader>();
		rawrbox::RESOURCES::addLoader<rawrbox::FontLoader>();
		// --------------

		render->init();
	}

	void Game::loadContent() {
		std::vector<std::pair<std::string, uint32_t>> initialContentFiles = {
		    {"./assets/models/shape_keys/shape_keys.glb", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_BLEND_SHAPES | rawrbox::ModelLoadFlags::Debug::PRINT_BLENDSHAPES},
		    {"./assets/models/ps1_phasmophobia/Phasmaphobia_Semi.fbx", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_LIGHT},
		    {"./assets/models/wolf/wolfman_animated.fbx", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS | rawrbox::ModelLoadFlags::Debug::PRINT_METADATA | rawrbox::ModelLoadFlags::Debug::PRINT_ANIMATIONS},
		    {"./assets/models/multiple_skeleton/twocubestest.gltf", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS | rawrbox::ModelLoadFlags::Debug::PRINT_BONE_STRUCTURE},
		    {"./assets/models/grandma_tv/scene.gltf", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS | rawrbox::ModelLoadFlags::Debug::PRINT_MATERIALS}};

		rawrbox::RESOURCES::loadListAsync(initialContentFiles, [this]() {
			rawrbox::runOnRenderThread([this]() {
				rawrbox::BindlessManager::processBarriers(); // IMPORTANT: BARRIERS NEED TO BE PROCESSED AFTER LOADING ALL THE CONTENT
				this->contentLoaded();
			});
		});
	}

	void Game::contentLoaded() {
		if (this->_ready) return;
		// Assimp test ---
		auto mdl = rawrbox::RESOURCES::getFile<rawrbox::ResourceAssimp>("./assets/models/ps1_phasmophobia/Phasmaphobia_Semi.fbx")->get();

		this->_model->setPos({7, 1.1F, 2.F});
		this->_model->load(*mdl);
		this->_model->upload();

		this->_model2->setPos({-6, 1.1F, -2.F});
		this->_model2->load(*mdl);
		this->_model2->upload();

		// ANIMATIONS ---
		auto mdl2 = rawrbox::RESOURCES::getFile<rawrbox::ResourceAssimp>("./assets/models/wolf/wolfman_animated.fbx")->get();
		this->_model3->load(*mdl2);
		this->_model3->playAnimation("Scene", true, 1.F);
		this->_model3->setPos({1, 0, 0});
		this->_model3->upload();

		this->_model4->load(*mdl2);
		this->_model4->playAnimation("Scene", true, 1.F);
		this->_model4->setPos({-1, 0, 0});
		this->_model4->upload();

		auto mdl4 = rawrbox::RESOURCES::getFile<rawrbox::ResourceAssimp>("./assets/models/multiple_skeleton/twocubestest.gltf")->get();
		this->_model5->load(*mdl4);
		this->_model5->playAnimation("MewAction", true, 0.8F);
		this->_model5->playAnimation("MewAction.001", true, 0.5F);
		this->_model5->setPos({0, 0, 2.5F});
		this->_model5->setScale({0.25F, 0.25F, 0.25F});
		this->_model5->upload();

		auto mdl5 = rawrbox::RESOURCES::getFile<rawrbox::ResourceAssimp>("./assets/models/grandma_tv/scene.gltf")->get();
		this->_model6->load(*mdl5);
		this->_model6->playAnimation("Scene", true, 1.F);
		this->_model6->setPos({0, 0, -3.5F});
		this->_model6->setScale({0.35F, 0.35F, 0.35F});
		this->_model6->setEulerAngle({0, rawrbox::MathUtils::toRad(180.F), 0});
		this->_model6->upload();

		auto mdl6 = rawrbox::RESOURCES::getFile<rawrbox::ResourceAssimp>("./assets/models/shape_keys/shape_keys.glb")->get();
		this->_model7->load(*mdl6);
		this->_model7->setScale({0.4F, 0.4F, 0.4F});
		this->_model7->setPos({2.F, 0.4F, -6.F});
		this->_model7->upload(true);
		//    -----

		// Text test ----
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "TEXTURES + LIGHT", {-6.F, 3.0F, 0});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "TEXTURES", {6.F, 3.0F, 0});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "SINGLE ARMATURE +\nVERTEX ANIMATION", {0.F, 2.F, 0});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "TWO ARMATURES +\nTWO ANIMATIONS", {0.F, 1.F, 2.5F});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "VERTEX ANIMATIONS", {0.F, 1.8F, -3.5F});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "EMBEDDED TEXTURES +\nBLEND SHAPES", {2.F, 1.8F, -6.F});
		this->_text->upload();
		// ------

		{
			auto mesh = rawrbox::MeshUtils::generateGrid(24, {0.F, 0.F, 0.F});
			this->_modelGrid->addMesh(mesh);
			this->_modelGrid->upload();
		}

		// LIGHT ----
		rawrbox::LIGHTS::add<rawrbox::PointLight>(rawrbox::Vector3f{-1, 1.6F, -1.4F}, rawrbox::Colors::White() * 100, 1.6F);
		rawrbox::LIGHTS::add<rawrbox::PointLight>(rawrbox::Vector3f{-1, 0.6F, -1.4F}, rawrbox::Colors::White() * 100, 1.6F);
		// -----------

		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) {
			rawrbox::Window::shutdown();
		} else {
			this->_model.reset();
			this->_model2.reset();
			this->_model3.reset();
			this->_model4.reset();
			this->_model5.reset();
			this->_model6.reset();
			this->_model7.reset();
			this->_modelGrid.reset();

			this->_text.reset();

			rawrbox::RESOURCES::shutdown();
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
		this->_modelGrid->draw();

		//
		this->_model->draw();
		this->_model2->draw();
		this->_model3->draw();
		this->_model4->draw();
		this->_model5->draw();
		this->_model6->draw();
		////

		this->_model7->setBlendShape("Cheese-Melt", std::abs(std::cos(rawrbox::FRAME * 0.005F) * 1.F));
		this->_model7->setBlendShape("Other-Nya", std::abs(std::cos(rawrbox::FRAME * 0.008F) * 1.F));
		this->_model7->setBlendShape("Other-Melt", std::abs(std::cos(rawrbox::FRAME * 0.002F) * 1.F));
		this->_model7->draw();
		//

		this->_text->draw();
	}

	void Game::draw() {
		rawrbox::Window::render(); // Commit primitives
	}
} // namespace assimp
