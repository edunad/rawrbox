

#include <rawrbox/engine/static.hpp>
#include <rawrbox/math/pi.hpp>
#include <rawrbox/render/cameras/orbit.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/timer.hpp>
#include <rawrbox/utils/vfs.hpp>

#include <instance_test/game.hpp>

#include <array>
#include <cmath>
#include <random>

namespace instance_test {
	void Game::setupGLFW() {
#if defined(_DEBUG) && defined(RAWRBOX_SUPPORT_DX12)
		auto* window = rawrbox::Window::createWindow(Diligent::RENDER_DEVICE_TYPE_D3D12); // DX12 is faster on DEBUG than Vulkan, due to vulkan having extra check steps to prevent you from doing bad things
#else
		auto* window = rawrbox::Window::createWindow();
#endif
		window->setMonitor(-1);
		window->setTitle("INSTANCE TEST");
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
		this->_camera = render->createCamera<rawrbox::CameraOrbit>(*window);
		this->_camera->setPos({0.F, 5.F, -5.F});
		this->_camera->setAngle({0.F, rawrbox::MathUtils::toRad(-45), 0.F, 0.F});
		this->_camera->canUseKeyboard([]() { return true; });
		this->_camera->canUseMouse([]() { return true; });
		// --------------

		// BINDS ----
		window->onKey += [this](rawrbox::Window& /*w*/, uint32_t key, uint32_t /*scancode*/, uint32_t action, uint32_t /*mods*/) {
			if (!this->_ready || action != rawrbox::KEY_ACTION_UP) return;
#ifdef _DEBUG
			if (key == rawrbox::KEY_F1) this->_camera->setCullFrozen(!this->_camera->isCullFrozen());
#endif
			if (key == rawrbox::KEY_F2) rawrbox::RENDERER->occlusion()->setEnabled(!rawrbox::RENDERER->occlusion()->isEnabled());
		};
		// --------------

		// Mount content
		rawrbox::VFS::mount("pak01_dir.vpk");
		// ----------

		// Add loaders
		rawrbox::RESOURCES::addLoader<rawrbox::TextureLoader>();
		// --------------

		// Pre-compile shaders before content load
		render->precompile<rawrbox::MaterialInstanced>();
		// ---------------------------------------

		render->init();
	}

	void Game::loadContent() {
		std::vector<std::pair<std::string, uint32_t>> initialContentFiles = {
		    {"./assets/textures/instance_test.png", 64},
		};

		rawrbox::RESOURCES::loadListAsync(initialContentFiles, [this]() {
			rawrbox::runOnRenderThread([this]() {
				this->contentLoaded();
			});
		});
	}

	void Game::contentLoaded() {
		if (this->_ready) return;

		int total = 1000;
		float spacing = 0.85F;

		auto* t = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/instance_test.png")->get();
		auto mesh = rawrbox::MeshUtils::generateCube({0, 0, 0}, {0.5F, 0.5F, 0.5F});
		mesh.setTexture(t);

		this->_model->setTemplate(mesh);

		std::random_device prng;
		std::uniform_int_distribution<uint16_t> dist(0, 4);
		std::uniform_real_distribution<float> distRot(0, 360);

		for (int z = 0; z < total; z++) {
			for (int x = 0; x < total; x++) {
				rawrbox::Matrix4x4 m;
				m.SRT({1.F, 1.F, 1.F}, rawrbox::Vector4f::toQuat({0, distRot(prng), 0}), {-500 + x * spacing, dist(prng) * 0.5F, -500 + z * spacing});
				this->_model->addInstance({m, rawrbox::Colors::White(), dist(prng)});
			}
		}

		this->_model->upload();

		// DYNAMIC TEMPLATE ---
		this->_texture = t;

		this->_dynamic->setTemplate({});
		this->_dynamic->reserve(this->DYNAMIC_INSTANCES);
		this->_dynamic->upload(rawrbox::UploadType::RESIZABLE_DYNAMIC);
		this->_dynamic->setPos({0.F, 3.5F, 2.F});
		// -------------

		this->_ready = true;
	}

	void Game::updateDynamic() {
		if (!this->_ready) return;
		this->_dynamicTime += rawrbox::DELTA_TIME;

		const auto shape = static_cast<size_t>(this->_dynamicTime) % 3U;
		if (shape != this->_dynamicShape) {
			this->_dynamicShape = shape;

			rawrbox::Mesh<> mesh = {};
			if (shape == 1U) mesh = rawrbox::MeshUtils::generateCube({0, 0, 0}, {0.5F, 0.5F, 0.5F});
			if (shape == 2U) mesh = rawrbox::MeshUtils::generateSphere({0, 0, 0}, {0.6F, 0.6F, 0.6F}, 1.F);
			if (!mesh.empty()) mesh.setTexture(this->_texture);

			this->_dynamic->setTemplate(mesh);
		}

		this->_dynamicInstances.clear();
		for (size_t i = 0; i < this->DYNAMIC_INSTANCES; i++) {
			const float angle = this->_dynamicTime * 0.5F + (static_cast<float>(i) / this->DYNAMIC_INSTANCES) * rawrbox::pi<float> * 2.F;

			rawrbox::Matrix4x4 m;
			m.SRT({0.6F, 0.6F, 0.6F}, rawrbox::Vector4f::toQuat({0, angle, 0}), {std::cos(angle) * 3.F, 0.F, std::sin(angle) * 3.F});
			this->_dynamicInstances.emplace_back(m, rawrbox::Colors::White(), static_cast<uint16_t>(i % 5U));
		}

		this->_dynamic->setInstances(std::move(this->_dynamicInstances));
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_RENDER) {
			this->_model.reset();
			this->_dynamic.reset();
			rawrbox::RESOURCES::shutdown();
		}

		rawrbox::Window::shutdown(thread);
	}

	void Game::pollEvents() {
		rawrbox::Window::pollEvents();
	}

	void Game::update() {
		rawrbox::Window::update();
		this->updateDynamic();
	}

	void Game::drawWorld() {
		if (!this->_ready || this->_model == nullptr) return;

		this->_model->draw();
		this->_dynamic->draw();
	}

	void Game::drawOverlay() const {
		auto* stencil = rawrbox::RENDERER->stencil();
		stencil->drawText(fmt::format("FPS: {}", this->getCurrentFPS()), {static_cast<float>(stencil->getSize().x) - 15.F, 15.F}, rawrbox::Colors::White(), rawrbox::Colors::Black(), rawrbox::Alignment::Right);

		if (!this->_ready) return;

#ifdef _DEBUG
		stencil->drawText(fmt::format("[F1]   FREEZE CULL: {}", this->_camera->isCullFrozen()), {15, 15});
#endif
		stencil->drawText(fmt::format("[F2]   OCCLUSION: {}", rawrbox::RENDERER->occlusion()->isEnabled()), {15, 30});

		constexpr std::array<const char*, 3> shapes = {"NONE", "CUBE", "SPHERE"};
		stencil->drawText(fmt::format("DYNAMIC TEMPLATE: {}", shapes[this->_dynamicShape % shapes.size()]), {15, 45});
	}

	void Game::draw() {
		rawrbox::Window::render(); // Commit primitives
	}
} // namespace instance_test