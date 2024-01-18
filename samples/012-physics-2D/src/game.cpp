
#include <rawrbox/engine/static.hpp>
#include <rawrbox/physics_2d/manager.hpp>
#include <rawrbox/physics_2d/utils.hpp>
#include <rawrbox/render/cameras/orbital.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/texture.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>

#include <phys_2d_test/game.hpp>

namespace phys_2d_test {
	void Game::setupGLFW() {
#ifdef _DEBUG
		auto window = rawrbox::Window::createWindow(Diligent::RENDER_DEVICE_TYPE_D3D12); // DX12 is faster on DEBUG than Vulkan, due to vulkan having extra check steps to prevent you from doing bad things
#else
		auto window = rawrbox::Window::createWindow();
#endif
		window->setMonitor(-1);
		window->setTitle("2D PHYSICS TEST");
		window->init(1024, 768, rawrbox::WindowFlags::Window::WINDOWED);
		window->onWindowClose += [this](auto& /*w*/) { this->shutdown(); };
	}

	void Game::init() {
		auto window = rawrbox::Window::getWindow();

		// Setup renderer
		auto render = window->createRenderer();
		render->onIntroCompleted = [this]() { this->loadContent(); };
		render->setDrawCall([this](const rawrbox::DrawPass& pass) {
			if (pass != rawrbox::DrawPass::PASS_OPAQUE) return;
			this->drawWorld();
		});
		// ---------------

		// Setup camera
		auto cam = render->setupCamera<rawrbox::CameraOrbital>(*window);
		cam->setPos({0.F, 5.F, -5.F});
		cam->setAngle({0.F, 0.F, 0.F, 0.F});
		// --------------

		// Setup physics
		rawrbox::PHYSICS_2D::physSettings->apply_gravity = false;
		rawrbox::PHYSICS_2D::init();
		/*rawrbox::PHYSICS_2D::onContact += [](const muli::ContactManifold& contact) {
			fmt::print("[RawrBox-Physics2D] Body {} contacted with body {}\n", contact.contactPoints[0].id, contact.contactPoints[1].id);
		};*/
		// ----

		// Add loaders
		rawrbox::RESOURCES::addLoader<rawrbox::TextureLoader>();
		// --------------

		// BINDS ----
		window->onMouseKey += [this](auto&, const rawrbox::Vector2i&, int button, int action, int) {
			const bool isDown = action == 1;
			if (!isDown || button != MOUSE_BUTTON_1) return;

			this->_paused = !this->_paused;
			if (this->_timer != nullptr) this->_timer->pause(this->_paused);
		};
		// -----

		render->init();
	}

	void Game::loadContent() {
		std::vector<std::pair<std::string, uint32_t>> initialContentFiles = {
		    {"./assets/textures/crate_hl1.png", 64}};

		rawrbox::RESOURCES::loadListAsync(initialContentFiles, [this]() {
			rawrbox::runOnRenderThread([this]() {
				rawrbox::BindlessManager::processBarriers(); // IMPORTANT: BARRIERS NEED TO BE PROCESSED AFTER LOADING ALL THE CONTENT
				this->contentLoaded();
			});
		});
	}

	void Game::contentLoaded() {
		if (this->_ready) return;

		// Setup grid --
		{
			auto mesh = rawrbox::MeshUtils::generateGrid(12, {0.F, 0.F, 0.F});
			this->_modelGrid->addMesh(mesh);
		}

		this->_modelGrid->setPos({0, 5.F, 0.1F});
		this->_modelGrid->setEulerAngle({rawrbox::MathUtils::toRad(90), 0, 0});
		this->_modelGrid->upload();

		this->_texture = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/crate_hl1.png")->get();

		// TIMER ---
		this->_timer = rawrbox::TIMER::create(
		    300, 25, [this]() { this->createBox({0, 5, 0}, {0.5F, 0.5F}); });
		this->_timer->pause(this->_paused);
		// --------

		this->_ready = true;
	}

	void Game::createBox(const rawrbox::Vector3f& pos, const rawrbox::Vector2f& size) {
		if (this->_texture == nullptr) return;

		auto box = std::make_unique<BoxOfDoom>();
		box->body = rawrbox::PHYSICS_2D::physWorld->CreateBox(size.x, size.y, muli::RigidBody::Type::dynamic_body);
		box->body->SetFriction(0.2F);
		box->body->SetPosition(rawrbox::Phys2DUtils::vecToPos(pos.xy()));

		// Create model
		box->mdl = std::make_unique<rawrbox::Model<>>();

		auto mesh = rawrbox::MeshUtils::generatePlane({}, size);
		mesh.setTexture(this->_texture);

		box->mdl->addMesh(mesh);
		box->mdl->upload();

		// Store reference
		this->_boxes.push_back(std::move(box));
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) {
			rawrbox::Window::shutdown();
		} else {
			this->_modelGrid.reset();
			this->_boxes.clear();

			this->_texture = nullptr;
			this->_timer = nullptr;

			rawrbox::PHYSICS_2D::shutdown();
			rawrbox::RESOURCES::shutdown();
		}
	}

	void Game::pollEvents() {
		rawrbox::Window::pollEvents();
	}

	void Game::update() {
		rawrbox::Window::update();
	}

	void Game::fixedUpdate() {
		if (this->_paused) return;
		rawrbox::PHYSICS_2D::tick();
	}

	void Game::drawWorld() {
		if (!this->_ready) return;
		if (this->_modelGrid != nullptr) this->_modelGrid->draw();

		// Draw boxes ----
		for (auto& b : this->_boxes) {
			auto body = b->body;
			if (body == nullptr) continue;

			auto pos = rawrbox::Phys2DUtils::posToVec(body->GetPosition());

			b->mdl->setPos(pos);
			b->mdl->setEulerAngle({0, 0, body->GetAngle()});
			b->mdl->setColor(body->IsSleeping() ? rawrbox::Colors::Gray() : rawrbox::Colors::White());

			b->mdl->draw();
		}
		// ------------------
	}

	void Game::draw() {
		rawrbox::Window::render(); // Commit primitives
	}
}; // namespace phys_2d_test
