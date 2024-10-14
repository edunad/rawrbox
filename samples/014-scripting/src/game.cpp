#include <rawrbox/render/lights/directional.hpp>
#include <rawrbox/render/scripting/plugin.hpp>
#include <rawrbox/resources/scripting/plugin.hpp>

#ifdef RAWRBOX_BASS
	#include <rawrbox/bass/resources/sound.hpp>
	#include <rawrbox/bass/scripting/plugin.hpp>
#endif

#ifdef RAWRBOX_NETWORK
	#include <rawrbox/network/scripting/plugin.hpp>
#endif

#ifdef RAWRBOX_UI
	#include <rawrbox/ui/scripting/plugin.hpp>
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
#if defined(_DEBUG) && defined(RAWRBOX_SUPPORT_DX12)
		auto* window = rawrbox::Window::createWindow(Diligent::RENDER_DEVICE_TYPE_D3D12); // DX12 is faster on DEBUG than Vulkan, due to vulkan having extra check steps to prevent you from doing bad things
#else
		auto* window = rawrbox::Window::createWindow();
#endif
		window->setMonitor(-1);
		window->setTitle("SCRIPTING TEST");
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
		render->onIntroCompleted = [this]() { this->loadContent(); };
		render->setDrawCall([this](const rawrbox::CameraBase& /*camera*/, const rawrbox::DrawPass& pass) {
			if (pass == rawrbox::DrawPass::PASS_WORLD) {
				this->drawWorld();
			} else {
				this->drawOverlay();
			}
		});
		// ---------------

		// Setup camera
		auto* cam = render->createCamera<rawrbox::CameraOrbital>(*window);
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
		rawrbox::SCRIPTING::registerPlugin<rawrbox::UIPlugin>(this->_ROOT_UI.get());
#endif
		// ----

		// Setup scripting
		rawrbox::SCRIPTING::registerPlugin<rawrbox::ResourcesPlugin>();

#ifdef RAWRBOX_BASS
		rawrbox::SCRIPTING::registerPlugin<rawrbox::BASScripting>();
#endif
#ifdef RAWRBOX_NETWORK
		rawrbox::SCRIPTING::registerPlugin<rawrbox::NetworkScripting>();
#endif

		rawrbox::SCRIPTING::registerPlugin<rawrbox::RendererScripting>(window);

		// Custom non-plugin ---
		rawrbox::SCRIPTING::registerPlugin<rawrbox::TestPlugin>();
		rawrbox::SCRIPTING::onRegisterGlobals += [this](rawrbox::Mod& mod) {
			auto* L = mod.getEnvironment();

			luabridge::getGlobalNamespace(L)
			    .addFunction("test_model", [this]() {
				    return this->_model.get();
			    })
			    .addFunction("test_model2", [this]() {
				    return this->_instance.get();
			    });
		};
		// ----

		rawrbox::SCRIPTING::setConsole(this->_console.get());
		rawrbox::SCRIPTING::init(2000); // Check files every 2 seconds

		// Load lua mods
		if (!std::filesystem::exists("./mods")) {
			std::filesystem::create_directory("./mods");
		}

		rawrbox::SCRIPTING::loadMods("./mods"); // To load the entire folder
		// rawrbox::SCRIPTING::loadMod("./mods/test_mod"); // To load a specific mod
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
				this->contentLoaded();
			});
		});
		// -----
	}

	void Game::contentLoaded() {
		if (this->_ready) return;
		auto* tex = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/crate_hl1.png")->get();
		this->_model->setMergeable(false);

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
		this->_model->upload();

		this->_instance->setTemplate(rawrbox::MeshUtils::generateCube({0.F, 0.0F, 0.F}, {0.1F, 0.1F, 0.1F}));
		this->_instance->upload();

		this->_ready = true;
		rawrbox::SCRIPTING::call("onReady");
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_RENDER) {
			this->_model.reset();
			this->_instance.reset();
			this->_console.reset();

#ifdef RAWRBOX_UI
			this->_ROOT_UI.reset();
#endif
			rawrbox::SCRIPTING::shutdown();
			rawrbox::RESOURCES::shutdown();
		}

		rawrbox::Window::shutdown(thread);
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
		rawrbox::SCRIPTING::call("draw", static_cast<int>(rawrbox::DrawPass::PASS_WORLD));
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
