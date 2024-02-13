#include <rawrbox/render/lights/directional.hpp>
#include <rawrbox/render/plugins/clustered.hpp>
#include <rawrbox/render/scripting/plugin.hpp>

#ifdef RAWRBOX_BASS
	#include <rawrbox/bass/resources/sound.hpp>
	#include <rawrbox/bass/scripting/plugin.hpp>
#endif

#ifdef RAWRBOX_NETWORK
	#include <rawrbox/network/scripting/plugin.hpp>
#endif

#ifdef RAWRBOX_UI
// #include <rawrbox/ui/scripting/plugin.hpp>
	#include <rawrbox/ui/static.hpp>
#endif

#include <rawrbox/render/cameras/orbital.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/scripting/manager.hpp>
#include <rawrbox/utils/timer.hpp>

#include <scripting_test/game.hpp>
#include <scripting_test/wrapper_test.hpp>

namespace scripting_test {
	void Game::setupGLFW() {
#ifdef _DEBUG
		auto window = rawrbox::Window::createWindow(Diligent::RENDER_DEVICE_TYPE_D3D12); // DX12 is faster on DEBUG than Vulkan, due to vulkan having extra check steps to prevent you from doing bad things
#else
		auto window = rawrbox::Window::createWindow();
#endif
		window->setMonitor(-1);
		window->setTitle("SCRIPTING TEST");
		window->init(1024, 768, rawrbox::WindowFlags::Window::WINDOWED);
		window->onWindowClose += [this](auto& /*w*/) { this->shutdown(); };
	}

	void Game::init() {
		auto window = rawrbox::Window::getWindow();

		// Setup renderer
		auto render = window->createRenderer();
		render->skipIntros(true);
		render->addPlugin<rawrbox::ClusteredPlugin>();
		render->onIntroCompleted = [this]() { this->loadContent(); };
		render->setDrawCall([this](const rawrbox::DrawPass& pass) {
			if (pass == rawrbox::DrawPass::PASS_OPAQUE) {
				this->drawWorld();
			} else {
				this->drawOverlay();
			}
		});
		// ---------------

		// Setup camera
		auto cam = render->setupCamera<rawrbox::CameraOrbital>(*window);
		cam->setPos({0.F, 5.F, -5.F});
		cam->setAngle({0.F, rawrbox::MathUtils::toRad(-45), 0.F, 0.F});
		// --------------

		// Add loaders
		rawrbox::RESOURCES::addLoader<rawrbox::TextureLoader>();
#ifdef RAWRBOX_BASS
		rawrbox::RESOURCES::addLoader<rawrbox::BASSLoader>();
#endif
		rawrbox::RESOURCES::addLoader<rawrbox::FontLoader>();
		//  --------------

		// SETUP UI
#ifdef RAWRBOX_UI
		this->_ROOT_UI = std::make_unique<rawrbox::UIRoot>(*window);
		// rawrbox::SCRIPTING::registerPlugin<rawrbox::UIPlugin>(this->_ROOT_UI.get());
#endif
		// ----

		// Setup scripting
#ifdef RAWRBOX_BASS
		rawrbox::SCRIPTING::registerPlugin<rawrbox::BASScripting>();
#endif
#ifdef RAWRBOX_NETWORK
		rawrbox::SCRIPTING::registerPlugin<rawrbox::NetworkScripting>();
#endif

		rawrbox::SCRIPTING::registerPlugin<rawrbox::RendererScripting>(window);

		// Custom non-plugin ---
		/*rawrbox::SCRIPTING::registerType<rawrbox::TestWrapper>();
		rawrbox::SCRIPTING::onRegisterGlobals += [this](rawrbox::Mod* mod) {
			mod->getEnvironment()["test"] = rawrbox::TestWrapper();
			mod->getEnvironment()["test_model"] = [this]() -> sol::object {
				if (!this->_ready || this->_model == nullptr) return sol::nil;
				return this->_model->getScriptingWrapper();
			};

			mod->getEnvironment()["test_model2"] = [this]() -> sol::object {
				if (!this->_ready || this->_instance == nullptr) return sol::nil;
				return this->_instance->getScriptingWrapper();
			};
		};*/
		// ----

		// ----

		rawrbox::SCRIPTING::setConsole(this->_console.get());
		rawrbox::SCRIPTING::init(2000); // Check files every 2 seconds

		// Load lua mods
		rawrbox::SCRIPTING::load();
		rawrbox::SCRIPTING::call("init");
		// ----

		render->init();
	}

	void Game::loadContent() {
		std::vector<std::pair<std::string, uint32_t>> initialContentFiles = {
		    {"./assets/textures/crate_hl1.png", 0}};

#ifdef RAWRBOX_UI
		initialContentFiles.insert(initialContentFiles.begin(), rawrbox::UI_RESOURCES.begin(), rawrbox::UI_RESOURCES.end()); // Insert the UI resources
#endif
		for (auto& f : initialContentFiles) {
			rawrbox::RESOURCES::preLoadFile(f.first, f.second);
		}

		// Load pre-content mod stuff ---
		rawrbox::SCRIPTING::call("onLoad");
		// ---

		// Start loading ----
		rawrbox::RESOURCES::startPreLoadQueueAsync(nullptr, nullptr, [this]() {
			rawrbox::runOnRenderThread([this]() {
				rawrbox::BindlessManager::processBarriers(); // IMPORTANT: BARRIERS NEED TO BE PROCESSED AFTER LOADING ALL THE CONTENT
				this->contentLoaded();
			});
		});
		// -----
	}

	void Game::contentLoaded() {
		if (this->_ready) return;
		auto tex = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/crate_hl1.png")->get();
		this->_model->setOptimizable(false);

		/*{
			auto mesh = rawrbox::MeshUtils::generateCube({0.F, 1.0F, 0.F}, {1.F, 1.F, 1.F});
			mesh.setTexture(tex);
			this->_model->addMesh(mesh);
		}*/

		{
			auto mesh = rawrbox::MeshUtils::generateGrid(12, {0.F, 0.F, 0.F});
			this->_model->addMesh(mesh);
		}
		// ----

		{
			auto mesh = rawrbox::MeshUtils::generateCube<rawrbox::MaterialInstancedLit>({0, 0, 0}, {2.1F, 2.1F, 2.1F});

			mesh.setEulerAngle({rawrbox::MathUtils::toRad(90), 0, 0});
			mesh.setTexture(tex);

			this->_instance->setTemplate(mesh);
		}

		this->_instance->addInstance({rawrbox::Matrix4x4::mtxSRT({1, 1, 1}, {}, {})});
		this->_instance->upload();

		this->_model->upload();

		// TEST LIGHT ---
		rawrbox::LIGHTS::add<rawrbox::DirectionalLight>(rawrbox::Vector3f{0.F, 10.F, 0}, rawrbox::Vector3f{0.F, -1.F, 0.F}, rawrbox::Colors::White()); // SUN
		// ---

		rawrbox::SCRIPTING::call("onReady");
		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) {
			rawrbox::Window::shutdown();
		} else {
#ifdef RAWRBOX_UI
			this->_ROOT_UI.reset();
#endif
			rawrbox::SCRIPTING::shutdown();
			rawrbox::RESOURCES::shutdown();
		}
	}

	void Game::pollEvents() {
		rawrbox::Window::pollEvents();
	}

	void Game::update() {
		rawrbox::Window::update();
		rawrbox::SCRIPTING::call("update");
	}

	void Game::fixedUpdate() {
		rawrbox::SCRIPTING::call("fixedUpdate");
	}

	void Game::drawWorld() {
		if (!this->_ready) return;
		if (this->_model != nullptr) this->_model->draw();
		if (this->_instance != nullptr) this->_instance->draw();
		rawrbox::SCRIPTING::call("draw", static_cast<int>(rawrbox::DrawPass::PASS_OPAQUE));
	}

	void Game::drawOverlay() {
		if (!this->_ready) return;
		rawrbox::SCRIPTING::call("draw", static_cast<int>(rawrbox::DrawPass::PASS_OVERLAY));

#ifdef RAWRBOX_UI
		this->_ROOT_UI->render();
#endif
	}

	void Game::draw() {
		rawrbox::Window::render(); // Draw world, overlay & commit primitives
	}
} // namespace scripting_test
