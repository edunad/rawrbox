

#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/cameras/orbital.hpp>
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
		window->setMonitor(1);
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

// --------------
#ifdef _DEBUG
		rawrbox::MaterialSkinned::DEBUG_MODE = false;
#endif

		render->init();
	}

	void Game::loadContent() {
		/*std::vector<std::pair<std::string, uint32_t>> initialContentFiles = {};

		rawrbox::RESOURCES::loadListAsync(initialContentFiles, [this]() {
			rawrbox::runOnRenderThread([this]() {
				this->contentLoaded();
			});
		});*/

		this->contentLoaded();
	}

	void Game::contentLoaded() {
		if (this->_ready) return;

		// TEST ---
		this->_tst = std::make_unique<rawrbox::GLTFImporter>(
		    rawrbox::ModelLoadFlags::IMPORT_TEXTURES |
		    rawrbox::ModelLoadFlags::IMPORT_BLEND_SHAPES |
		    rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS |

		    rawrbox::ModelLoadFlags::Optimizer::MESH |
		    rawrbox::ModelLoadFlags::Optimizer::SKELETON_ANIMATIONS |

		    rawrbox::ModelLoadFlags::Debug::PRINT_BLENDSHAPES |
		    rawrbox::ModelLoadFlags::Debug::PRINT_BONE_STRUCTURE |
		    rawrbox::ModelLoadFlags::Debug::PRINT_ANIMATIONS);

		// this->_tst->load("./assets/models/grandma_tv/scene.gltf");
		this->_tst->load("./assets/models/813_kelley_road.glb");
		// this->_tst->load("./assets/models/shape_keys/shape_keys.glb");
		//  this->_tst->load("./assets/models/wolf/wolf.glb");
		//   this->_tst->load("./assets/models/skin_test.gltf");

		this->_tstMdl = std::make_unique<rawrbox::GLTFModel<rawrbox::MaterialSkinned>>();
		this->_tstMdl->setScale({0.005F, 0.005F, 0.005F});
		this->_tstMdl->load(*this->_tst);
		this->_tstMdl->playAnimation(true, 1.F);
		// this->_tstMdl->setWireframe(true);
		this->_tstMdl->upload(rawrbox::UploadType::STATIC);
		//  ----------

		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_RENDER) {
			this->_tst.reset();
			this->_tstMdl.reset();

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
		if (this->_tstMdl == nullptr) return;

		// this->_tstMdl->setBlendShape("Cheese-Melt", std::cos(this->_time) * 0.5F);
		// this->_tstMdl->setBlendShape("Cheese-Morph", std::cos(this->_time) * 0.5F);
		this->_tstMdl->draw();

		// this->_time += 0.01F;
	}

	void Game::draw() {
		rawrbox::Window::render(); // Commit primitives
	}
} // namespace gltf
