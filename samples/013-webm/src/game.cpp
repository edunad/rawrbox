
#include <rawrbox/render/cameras/orbital.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/webm/resources/webm.hpp>
#include <rawrbox/webm/textures/webm.hpp>

#include <webm_test/game.hpp>

#include <fmt/format.h>

namespace webm_test {
	void Game::setupGLFW() {
#if defined(_DEBUG) && defined(RAWRBOX_SUPPORT_DX12)
		auto* window = rawrbox::Window::createWindow(Diligent::RENDER_DEVICE_TYPE_D3D12); // DX12 is faster on DEBUG than Vulkan, due to vulkan having extra check steps to prevent you from doing bad things
#else
		auto* window = rawrbox::Window::createWindow();
#endif
		window->setMonitor(-1);
		window->setTitle("WEBM TEST");
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
		render->setDrawCall([this](const rawrbox::DrawPass& pass) {
			if (pass != rawrbox::DrawPass::PASS_OPAQUE) return;
			this->drawWorld();
		});
		// ---------------

		// Setup camera
		auto* cam = render->setupCamera<rawrbox::CameraOrbital>(*window);
		cam->setPos({0.F, 5.F, -10.F});
		cam->setAngle({0.F, 0.F, 0.F, 0.F});
		// --------------

		// Setup loaders
		rawrbox::RESOURCES::addLoader<rawrbox::TextureLoader>();
		rawrbox::RESOURCES::addLoader<rawrbox::WEBMLoader>();
		// ----------

		render->init();
	}

	void Game::loadContent() {
		std::vector<std::pair<std::string, uint32_t>> initialContentFiles = {
		    {"./assets/video/webm_test.webm", 0 | rawrbox::WEBMLoadFlags::PRELOAD},
		    {"./assets/video/webm_test_2.webm", 0 | rawrbox::WEBMLoadFlags::PRELOAD}};

		rawrbox::RESOURCES::loadListAsync(initialContentFiles, [this]() {
			rawrbox::runOnRenderThread([this]() {
				this->contentLoaded();
			});
		});
	}

	void Game::contentLoaded() {
		if (this->_ready) return;

		auto* tex = rawrbox::RESOURCES::getFile<rawrbox::ResourceWEBM>("./assets/video/webm_test.webm")->get<rawrbox::TextureWEBM>();
		auto* tex2 = rawrbox::RESOURCES::getFile<rawrbox::ResourceWEBM>("./assets/video/webm_test_2.webm")->get<rawrbox::TextureWEBM>();
		tex2->onEnd += [tex]() {
			fmt::print("WEBM reached end\n");
			tex->seek(6500);
		};

		{
			auto mesh = rawrbox::MeshUtils::generatePlane({-2.F, -4.0F, 0.F}, {4.F, 3.F});
			mesh.setTexture(tex);

			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generatePlane({2.F, -4.0F, 0.F}, {4.F, 7.F});
			mesh.setTexture(tex2);

			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateGrid(12, {0.F, 0.F, 0.F});
			this->_model->addMesh(mesh);
		}
		// ----

		this->_model->setEulerAngle({0, 0, rawrbox::MathUtils::toRad(180)});
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
	}

	void Game::drawWorld() {
		if (!this->_ready) return;
		if (this->_model != nullptr) this->_model->draw();
	}

	void Game::draw() {
		rawrbox::Window::render(); // Commit primitives
	}
} // namespace webm_test
