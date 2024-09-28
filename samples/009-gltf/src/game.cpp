

#include <rawrbox/gltf/importer.hpp>
#include <rawrbox/gltf/resources/model.hpp>
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/cameras/orbital.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
#include <rawrbox/render/plugins/clustered.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/threading.hpp>

#include <gltf/game.hpp>

namespace gltf {
	void Game::setupGLFW() {
#if defined(_DEBUG) && defined(RAWRBOX_SUPPORT_DX12)
		auto* window = rawrbox::Window::createWindow(Diligent::RENDER_DEVICE_TYPE_D3D12); // DX12 is faster on DEBUG than Vulkan, due to vulkan having extra check steps to prevent you from doing bad things
#else
		auto* window = rawrbox::Window::createWindow();
#endif
		window->setMonitor(-1);
		window->setTitle("GLTF TEST");
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
		render->onIntroCompleted = [this]() { this->loadContent(); };
		render->addPlugin<rawrbox::ClusteredPlugin>();
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

		// Add loaders
		rawrbox::RESOURCES::addLoader<rawrbox::GLTFLoader>();
		// --------------

#ifdef _DEBUG
		rawrbox::MaterialSkinned::DEBUG_MODE = false;
#endif

		render->init();
	}

	void Game::loadContent() {
		std::vector<std::pair<std::string, uint32_t>> initialContentFiles = {
		    {"./assets/models/ps1_phasmophobia/scene.glb", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_LIGHT | rawrbox::ModelLoadFlags::Optimizer::MESH},
		    {"./assets/models/shape_keys/shape_keys.glb", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_BLEND_SHAPES | rawrbox::ModelLoadFlags::Debug::PRINT_BLENDSHAPES},
		    {"./assets/models/wolf/wolf.glb", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS | rawrbox::ModelLoadFlags::Optimizer::SKELETON_ANIMATIONS | rawrbox::ModelLoadFlags::Debug::PRINT_ANIMATIONS},
		    {"./assets/models/anim_test.glb", rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS},
		    {"./assets/models/grandma_tv/scene.gltf", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS | rawrbox::ModelLoadFlags::Debug::PRINT_MATERIALS}};

		rawrbox::RESOURCES::loadListAsync(initialContentFiles, [this]() {
			rawrbox::runOnRenderThread([this]() {
				this->contentLoaded();
			});
		});
	}

	void Game::contentLoaded() {
		if (this->_ready) return;

		// PHASMO
		{
			auto* mdl = rawrbox::RESOURCES::getFile<rawrbox::ResourceGLTF>("./assets/models/ps1_phasmophobia/scene.glb")->get();

			this->_phasmo = std::make_unique<rawrbox::GLTFModel<rawrbox::MaterialLit>>();
			this->_phasmo->load(*mdl);

			this->_phasmo->setPos({7, 1.1F, 2.F});
			this->_phasmo->upload();
		}
		// -------

		// TV
		{
			auto* mdl = rawrbox::RESOURCES::getFile<rawrbox::ResourceGLTF>("./assets/models/grandma_tv/scene.gltf")->get();

			this->_grandmaTV = std::make_unique<rawrbox::GLTFModel<>>();
			this->_grandmaTV->load(*mdl);

			this->_grandmaTV->setScale({0.35F, 0.35F, 0.35F});
			this->_grandmaTV->setEulerAngle({0, rawrbox::MathUtils::toRad(180.F), 0});
			this->_grandmaTV->setPos({-1, 0, -3.5F});
			this->_grandmaTV->upload();

			{
				auto anims = this->_grandmaTV->playAnimation();
				for (auto& anim : anims)
					anim->setLoop(true);
			}
		}
		// ----------

		// BLEND SHAPE
		{
			auto* mdl = rawrbox::RESOURCES::getFile<rawrbox::ResourceGLTF>("./assets/models/shape_keys/shape_keys.glb")->get();

			this->_blendTest = std::make_unique<rawrbox::GLTFModel<>>();
			this->_blendTest->load(*mdl);

			this->_blendTest->setPos({1.F, 0.4F, -3.5F});
			this->_blendTest->upload(rawrbox::UploadType::FIXED_DYNAMIC);
		}
		// ----------

		// ANIMATION TEST
		{
			auto* mdl = rawrbox::RESOURCES::getFile<rawrbox::ResourceGLTF>("./assets/models/anim_test.glb")->get();

			this->_animTest = std::make_unique<rawrbox::GLTFModel<rawrbox::MaterialSkinned>>();
			this->_animTest->load(*mdl);

			this->_animTest->setScale({0.25F, 0.25F, 0.25F});
			this->_animTest->setPos({2.F, 0.F, 2.5F});
			this->_animTest->upload();
		}
		// ----------

		// WOLF ----
		{
			auto* mdl = rawrbox::RESOURCES::getFile<rawrbox::ResourceGLTF>("./assets/models/wolf/wolf.glb")->get();

			this->_wolf = std::make_unique<rawrbox::GLTFModel<rawrbox::MaterialSkinned>>();
			this->_wolfLit = std::make_unique<rawrbox::GLTFModel<rawrbox::MaterialSkinnedLit>>();

			this->_wolf->load(*mdl);
			this->_wolfLit->load(*mdl);

			this->_wolf->setPos({1, 0, 0});
			this->_wolf->upload();

			this->_wolfLit->setPos({-1, 0, 0});
			this->_wolfLit->upload();

			{
				auto anims = this->_wolf->playAnimation();
				for (auto& anim : anims)
					anim->setLoop(true);
			}

			{
				auto anims = this->_wolfLit->playAnimation();
				for (auto& anim : anims)
					anim->setLoop(true);
			}
		}
		// ----------

		{
			auto mesh = rawrbox::MeshUtils::generateGrid(24, {0.F, 0.F, 0.F});
			this->_modelGrid->addMesh(mesh);
			this->_modelGrid->upload();
		}

		// LIGHT ----
		rawrbox::LIGHTS::add<rawrbox::PointLight>(rawrbox::Vector3f{-1, 1.6F, -1.4F}, rawrbox::Colors::White() * 100, 1.6F);
		rawrbox::LIGHTS::add<rawrbox::PointLight>(rawrbox::Vector3f{-1, 0.6F, -1.4F}, rawrbox::Colors::White() * 100, 1.6F);
		// -----------

		// ANIM TEST ---
		this->playTestAnim();
		// -------------

		this->_ready = true;
	}

	void Game::playTestAnim() {
		if (this->_animTest == nullptr) return;

		this->_animTest->playAnimation("TEST.UP", true, [this](const std::string&) {
			if (this->_animTest == nullptr) return;

			auto* a = this->_animTest->playAnimation("TEST.LEFT", true, [this](const std::string&) {
				if (this->_animTest == nullptr) return;

				this->_animTest->playAnimation("TEST.BACK", true, [this](const std::string&) {
					this->playTestAnim();
				});
			});

			a->setSpeed(-1);
		});
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_RENDER) {
			this->_blendTest.reset();
			this->_grandmaTV.reset();
			this->_wolf.reset();
			this->_wolfLit.reset();
			this->_modelGrid.reset();
			this->_text.reset();
			this->_animTest.reset();
			this->_phasmo.reset();

			rawrbox::RESOURCES::shutdown();
		}

		rawrbox::Window::shutdown(thread);
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

		// TV ---
		if (this->_grandmaTV != nullptr) this->_grandmaTV->draw();
		// ---------

		// BLEND SHAPE ----
		if (this->_blendTest != nullptr) {
			this->_blendTest->setBlendShape("Cheese-Melt", std::abs(std::cos(rawrbox::FRAME * 0.005F) * 1.F));
			this->_blendTest->setBlendShape("Other-Nya", std::abs(std::cos(rawrbox::FRAME * 0.008F) * 1.F));
			this->_blendTest->setBlendShape("Other-Melt", std::abs(std::cos(rawrbox::FRAME * 0.002F) * 1.F));
			this->_blendTest->draw();
		}
		//  -----

		// WOLF ---
		if (this->_wolf != nullptr && this->_wolfLit != nullptr) {
			this->_wolf->draw();
			this->_wolfLit->draw();
		}
		// --------

		// ANIM TEST --
		if (this->_animTest != nullptr) this->_animTest->draw();
		// -------

		// LIGHT TEST --
		if (this->_phasmo != nullptr) this->_phasmo->draw();
		// -------------
	}

	void Game::draw() {
		rawrbox::Window::render(); // Commit primitives
	}
} // namespace gltf
