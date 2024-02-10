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
#endif
		// ----

		// Setup scripting
#ifdef RAWRBOX_BASS
		rawrbox::SCRIPTING::registerPlugin<rawrbox::BASSPlugin>();
#endif
#ifdef RAWRBOX_NETWORK
		rawrbox::SCRIPTING::registerPlugin<rawrbox::NetworkPlugin>();
#endif

		rawrbox::SCRIPTING::registerPlugin<rawrbox::RendererPlugin>(window);

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

		/*
				// Setup camera
				auto cam = this->_window->setupCamera<rawrbox::CameraOrbital>(*this->_window);
				cam->setPos({0.F, 5.F, -5.F});
				cam->setAngle({0.F, rawrbox::MathUtils::toRad(-45), 0.F, 0.F});
				// --------------

		#ifdef RAWRBOX_UI
				// SETUP UI
				this->_ROOT_UI = std::make_unique<rawrbox::UIRoot>(*this->_window);
				// ----
		#endif

				// Setup loaders
				rawrbox::RESOURCES::addLoader<rawrbox::TextureLoader>();
		#ifdef RAWRBOX_BASS
				rawrbox::RESOURCES::addLoader<rawrbox::BASSLoader>();
		#endif
				rawrbox::RESOURCES::addLoader<rawrbox::FontLoader>();
				// ----------

				// Setup scripting
				rawrbox::SCRIPTING::registerPlugin<rawrbox::RenderPlugin>(this->_window.get());
				rawrbox::SCRIPTING::registerPlugin<rawrbox::ResourcesPlugin>();

		#ifdef RAWRBOX_BASS
				rawrbox::SCRIPTING::registerPlugin<rawrbox::BASSPlugin>();
		#endif

		#ifdef RAWRBOX_NETWORK
				rawrbox::SCRIPTING::registerPlugin<rawrbox::NetworkPlugin>();
		#endif

		#ifdef RAWRBOX_UI
				rawrbox::SCRIPTING::registerPlugin<rawrbox::UIPlugin>(this->_ROOT_UI.get());
		#endif

				// Custom non-plugin ---
				rawrbox::SCRIPTING::registerType<rawrbox::TestWrapper>();
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
				};
				// ----

				rawrbox::SCRIPTING::init(2000); // Check files every 2 seconds

				// Load lua mods
				rawrbox::SCRIPTING::load();
				rawrbox::SCRIPTING::call("init");
				// ----
		*/
	}

	void Game::loadContent() {

		std::vector<std::pair<std::string, uint32_t>> initialContentFiles = {
		    {"./content/textures/crate_hl1.png", 0}};

#ifdef RAWRBOX_UI
		initialContentFiles.insert(initialContentFiles.begin(), rawrbox::UI_RESOURCES.begin(), rawrbox::UI_RESOURCES.end()); // Insert the UI resources
#endif

		rawrbox::RESOURCES::loadListAsync(initialContentFiles, [this]() {
			rawrbox::runOnRenderThread([this]() {
				rawrbox::BindlessManager::processBarriers(); // IMPORTANT: BARRIERS NEED TO BE PROCESSED AFTER LOADING ALL THE CONTENT
				this->contentLoaded();
			});
		});

		/*std::vector initialContentFiles = {
		    std::make_pair<std::string, uint32_t>("./content/textures/crate_hl1.png", 0)};

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
		this->_loadingFiles = static_cast<int>(rawrbox::RESOURCES::getTotalPreload());
		rawrbox::RESOURCES::startPreLoadQueueAsync(nullptr, [this](std::string f, uint32_t) {
			fmt::print("Loaded {}!\n", f);
			this->_loadingFiles--;
			if (this->_loadingFiles <= 0) {
				rawrbox::runOnRenderThread([this]() { this->contentLoaded(); });
			}
		});
		// -----*/
	}

	void Game::contentLoaded() {
		if (this->_ready) return;
		auto tex = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./content/textures/crate_hl1.png")->get();
		this->_model->setOptimizable(false);

		{
			auto mesh = rawrbox::MeshUtils::generateCube({0.F, 1.0F, 0.F}, {1.F, 1.F, 1.F});
			mesh.setTexture(tex);
			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateGrid(12, {0.F, 0.F, 0.F});
			this->_model->addMesh(mesh);
		}
		// ----

		this->_instance->setTemplate(rawrbox::MeshUtils::generateCube({0.F, 0.0F, 0.F}, {0.1F, 0.1F, 0.1F}));
		this->_instance->upload();

		this->_model->upload();
		this->_ready = true;

		// rawrbox::SCRIPTING::call("onReady");
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
	}

	void Game::drawWorld() {
		if (!this->_ready) return;
		if (this->_model != nullptr) this->_model->draw();
		if (this->_instance != nullptr) this->_instance->draw();
	}

	void Game::drawOverlay() {
		if (!this->_ready) return;
			// rawrbox::SCRIPTING::call("drawOverlay");

#ifdef RAWRBOX_UI
		this->_ROOT_UI->render();
#endif
	}

	void Game::draw() {
		rawrbox::Window::render(); // Draw world, overlay & commit primitives
	}
} // namespace scripting_test
