
#include <rawrbox/engine/static.hpp>
#include <rawrbox/physics/utils.hpp>
#include <rawrbox/render/cameras/orbital.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/texture.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>

#include <phys_3d_test/game.hpp>

namespace phys_3d_test {
	void Game::setupGLFW() {
#if defined(_DEBUG) && defined(RAWRBOX_SUPPORT_DX12)
		auto* window = rawrbox::Window::createWindow(Diligent::RENDER_DEVICE_TYPE_D3D12); // DX12 is faster on DEBUG than Vulkan, due to vulkan having extra check steps to prevent you from doing bad things
#else
		auto window = rawrbox::Window::createWindow();
#endif
		window->setMonitor(-1);
		window->setTitle("3D PHYSICS TEST");
#ifdef _DEBUG
		window->init(1024, 768, rawrbox::WindowFlags::Window::WINDOWED);
#else
		window->init(-1, -1, rawrbox::WindowFlags::Window::BORDERLESS);
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
		cam->setPos({0.F, 5.F, -5.F});
		cam->setAngle({0.F, rawrbox::MathUtils::toRad(-45), 0.F, 0.F});
		// --------------

		// Add loaders
		rawrbox::RESOURCES::addLoader<rawrbox::TextureLoader>();
		// --------------

		// Initialize physics
		rawrbox::PHYSICS::init(20, 2048, 2048, 2048, 2048, 5);

		auto settings = rawrbox::PHYSICS::physicsSystem->GetPhysicsSettings();
		settings.mNumPositionSteps = 1;
		settings.mNumVelocitySteps = 2;
		settings.mDeterministicSimulation = false;
		settings.mTimeBeforeSleep = 0.15F;

		rawrbox::PHYSICS::physicsSystem->SetPhysicsSettings(settings);
		// rawrbox::PHYSICS::onBodyAwake += [](const JPH::BodyID& id, uint64_t inBodyUserData) { fmt::print("Body awake \n"); };
		// rawrbox::PHYSICS::onBodySleep += [](const JPH::BodyID& id, uint64_t inBodyUserData) { fmt::print("Body sleep \n"); };
		// ----------------------------

		// BINDS ----
		window->onMouseKey += [this](auto&, const rawrbox::Vector2i&, int button, int action, int) {
			const bool isDown = action == 1;
			if (!isDown || button != rawrbox::MOUSE_BUTTON_1) return;

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
				this->contentLoaded();
			});
		});
	}

	void Game::contentLoaded() {
		if (this->_ready) return;
		// Setup phys world

		// The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
		// variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
		JPH::BodyInterface& body_interface = rawrbox::PHYSICS::physicsSystem->GetBodyInterface();

		// Next we can create a rigid body to serve as the floor, we make a large box
		// Create the settings for the collision volume (the shape).
		// Note that for simple shapes (like boxes) you can also directly construct a BoxShape.
		JPH::BoxShapeSettings floor_shape_settings(JPH::Vec3(100.0F, 0.1F, 100.0F));

		// Create the shape
		JPH::ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
		JPH::ShapeRefC floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()

		// Create the settings for the body itself. Note that here you can also set other properties like the restitution / friction.
		JPH::BodyCreationSettings floor_settings(floor_shape, JPH::RVec3(0.0_r, 0.0_r, 0.0_r), JPH::Quat::sIdentity(), JPH::EMotionType::Static, static_cast<JPH::ObjectLayer>(rawrbox::PHYS_LAYERS::STATIC));
		floor_settings.mFriction = 1.F;
		// ---

		// Create the actual rigid body
		JPH::Body* floor = body_interface.CreateBody(floor_settings); // Note that if we run out of bodies this can return nullptr

		// Add it to the world
		body_interface.AddBody(floor->GetID(), JPH::EActivation::DontActivate);
		// -----

		rawrbox::PHYSICS::optimize(); // Only need to be called after adding a lot of bodies in one go

		// Setup grid --
		{
			auto mesh = rawrbox::MeshUtils::generateGrid(24, {0.F, 0.F, 0.F});
			this->_modelGrid->addMesh(mesh);
			this->_modelGrid->upload();
		}
		// -------------

		this->_texture = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/crate_hl1.png")->get();

		// TIMER ---
		this->_timer = rawrbox::TIMER::create(
		    600, 25, [this]() { this->createBox({0, 5, 0}, {0.5F, 0.5F, 0.5F}); }, [this] {
			    this->_timer = nullptr;
			    rawrbox::PHYSICS::optimize(); // Only need to be called after adding a lot of bodies in one go
		    });

		this->_timer->pause(this->_paused);
		// --------

		this->_ready = true;
	}

	void Game::createBox(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& size) {
		if (this->_texture == nullptr) return;

		auto box = std::make_unique<BoxOfDoom>();

		JPH::BodyInterface& body_interface = rawrbox::PHYSICS::physicsSystem->GetBodyInterface();
		JPH::BoxShapeSettings box_shape_settings(JPH::Vec3(size.x / 2.F, size.y / 2.F, size.z / 2.F));
		// JPH::SphereShapeSettings box_shape_settings(size.x / 2.F);

		JPH::ShapeSettings::ShapeResult box_result = box_shape_settings.Create();
		JPH::ShapeRefC box_shape = box_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()

		// Create the settings for the body itself. Note that here you can also set other properties like the restitution / friction.
		JPH::BodyCreationSettings box_settings(box_shape, JPH::RVec3(pos.x, pos.y, pos.x), JPH::Quat::sIdentity(), JPH::EMotionType::Dynamic, static_cast<JPH::ObjectLayer>(rawrbox::PHYS_LAYERS::DYNAMIC));
		box_settings.mFriction = 1.F;
		// ---

		// Create the actual rigid body
		box->body = body_interface.CreateBody(box_settings);
		body_interface.AddBody(box->body->GetID(), JPH::EActivation::Activate);

		// Create model
		box->mdl = std::make_unique<rawrbox::Model<>>();

		auto mesh = rawrbox::MeshUtils::generateCube({}, size);
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

			rawrbox::PHYSICS::shutdown();
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
		rawrbox::PHYSICS::tick();
	}

	void Game::drawWorld() {
		if (!this->_ready) return;
		if (this->_modelGrid != nullptr) this->_modelGrid->draw();

		// Draw boxes ----
		for (auto& b : this->_boxes) {
			auto* body = b->body;
			if (body == nullptr) continue;

			auto pos = rawrbox::PhysUtils::posToVec(body->GetCenterOfMassPosition());
			auto ang = rawrbox::PhysUtils::quatToVec4(body->GetRotation());

			b->mdl->setPos(pos);
			b->mdl->setAngle(ang);
			b->mdl->setColor(body->IsActive() ? rawrbox::Colors::White() : rawrbox::Colors::Gray());

			b->mdl->draw();
		}
		// ------------------*/
	}

	void Game::draw() {
		rawrbox::Window::render(); // Commit primitives
	}
}; // namespace phys_3d_test
