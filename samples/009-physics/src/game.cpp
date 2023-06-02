
#include <rawrbox/physics/utils.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>

#include <physics_test/game.hpp>

#include <fmt/format.h>

#include <vector>

namespace physics_test {
	void Game::init() {
		int width = 1024;
		int height = 768;

		this->_window = std::make_shared<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("PHYSICS TEST");
		this->_window->setRenderer(bgfx::RendererType::Count);
		this->_window->initialize(width, height, rawrbox::WindowFlags::Debug::TEXT | rawrbox::WindowFlags::Debug::PROFILER | rawrbox::WindowFlags::Window::WINDOWED);
		this->_window->onWindowClose += [this](auto& w) {
			this->shutdown();
		};

		// Setup camera
		this->_camera = std::make_shared<rawrbox::CameraOrbital>(this->_window);
		this->_camera->setPos({0.F, 5.F, -5.F});
		this->_camera->setAngle({0.F, bx::toRad(-45), 0.F, 0.F});
		// --------------

		// Setup loaders
		rawrbox::RESOURCES::addLoader(std::make_unique<rawrbox::TextureLoader>());

		// Initialize physics
		rawrbox::PHYSICS::init();

		// Load content ---
		this->loadContent();
		// -----
	}

	void Game::loadContent() {
		std::array<std::string, 1> initialContentFiles = {
		    "content/textures/crate_hl1.png",
		};

		rawrbox::ASYNC::run([initialContentFiles]() {
			for (auto& f : initialContentFiles) {
				rawrbox::RESOURCES::loadFile(f, 0);
			} }, [this] { rawrbox::runOnMainThread([this]() {
										  rawrbox::RESOURCES::upload();
										  this->contentLoaded();
									  }); });

		this->_window->upload();
	}

	void Game::contentLoaded() {
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
		// ---

		// Create the actual rigid body
		JPH::Body* floor = body_interface.CreateBody(floor_settings); // Note that if we run out of bodies this can return nullptr

		// Add it to the world
		body_interface.AddBody(floor->GetID(), JPH::EActivation::DontActivate);
		// -----

		rawrbox::PHYSICS::optimize(); // Only need to be called after adding a lot of bodies in one go

		// Setup grid
		{
			auto mesh = this->_modelGrid->generateGrid(24, {0.F, 0.F, 0.F});
			this->_modelGrid->addMesh(mesh);
			this->_modelGrid->upload();
		}
		// -----

		// BINDS ----
		this->_window->onMouseKey += [this](auto& w, const rawrbox::Vector2i& mousePos, int button, int action, int mods) {
			const bool isDown = action == 1;
			if (!isDown || button != MOUSE_BUTTON_1) return;

			fmt::print("Create box\n");
			this->createBox({0, 5, 0}, {0.5F, 0.5F, 0.5F});
		};

		this->_ready = true;
	}

	void Game::createBox(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& size) {
		BoxOfDoom box;

		JPH::BodyInterface& body_interface = rawrbox::PHYSICS::physicsSystem->GetBodyInterface();
		JPH::BoxShapeSettings box_shape_settings(JPH::Vec3(size.x / 2.F, size.y / 2.F, size.z / 2.F));

		JPH::ShapeSettings::ShapeResult box_result = box_shape_settings.Create();
		JPH::ShapeRefC box_shape = box_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()

		// Create the settings for the body itself. Note that here you can also set other properties like the restitution / friction.
		JPH::BodyCreationSettings box_settings(box_shape, JPH::RVec3(pos.x, pos.y, pos.x), JPH::Quat::sIdentity(), JPH::EMotionType::Dynamic, static_cast<JPH::ObjectLayer>(rawrbox::PHYS_LAYERS::DYNAMIC));
		// ---

		// Create the actual rigid body
		box.body = body_interface.CreateBody(box_settings);
		body_interface.AddBody(box.body->GetID(), JPH::EActivation::Activate);

		// Create model
		box.mdl = std::make_shared<rawrbox::Model<>>();
		auto mesh = box.mdl->generateCube({}, size);

		auto text = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./content/textures/crate_hl1.png")->texture;
		mesh->setTexture(text);

		box.mdl->addMesh(mesh);
		box.mdl->upload();

		// Store reference
		this->_boxes.push_back(box);
	}

	void Game::shutdown() {
		this->_window.reset();
		this->_camera.reset();
		this->_modelGrid.reset();

		this->_boxes.clear();

		rawrbox::PHYSICS::shutdown();
		rawrbox::Engine::shutdown();
	}

	void Game::pollEvents() {
		if (this->_window == nullptr) return;
		this->_window->pollEvents();
	}

	void Game::update() {
		if (this->_window == nullptr) return;
		this->_window->update();
		this->_camera->update();

		for (auto& b : this->_boxes) {
			auto body = b.body;
			if (body == nullptr) continue;

			auto pos = rawrbox::PhysUtils::posToVec(body->GetCenterOfMassPosition());
			auto ang = rawrbox::PhysUtils::quatToVec4(body->GetRotation());

			b.mdl->setPos(pos);
			b.mdl->setAngle(ang);
		}

		rawrbox::PHYSICS::tick();
	}

	void printFrames() {
		const bgfx::Stats* stats = bgfx::getStats();

		bgfx::dbgTextPrintf(1, 4, 0x6f, "GPU %0.6f [ms]", double(stats->gpuTimeEnd - stats->gpuTimeBegin) * 1000.0 / stats->gpuTimerFreq);
		bgfx::dbgTextPrintf(1, 5, 0x6f, "CPU %0.6f [ms]", double(stats->cpuTimeEnd - stats->cpuTimeBegin) * 1000.0 / stats->cpuTimerFreq);
		bgfx::dbgTextPrintf(1, 6, 0x6f, fmt::format("TRIANGLES: {} ----->    DRAW CALLS: {}", stats->numPrims[bgfx::Topology::TriList], stats->numDraw).c_str());
	}

	void Game::drawWorld() {
		for (auto& b : this->_boxes) {
			b.mdl->draw(this->_camera->getPos());
		}

		this->_modelGrid->draw({});
	}

	void Game::draw() {
		if (this->_window == nullptr) return;
		this->_window->clear(); // Clean up and set renderer

		// DEBUG ----
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(1, 1, 0x1f, "009-physics");
		bgfx::dbgTextPrintf(1, 2, 0x3f, "Description: PHYSICS test");
		printFrames();
		// -----------

		if (!this->_ready) {
			bgfx::dbgTextPrintf(1, 10, 0x70, "                                   ");
			bgfx::dbgTextPrintf(1, 11, 0x70, "          LOADING CONTENT          ");
			bgfx::dbgTextPrintf(1, 12, 0x70, "                                   ");
		} else {
			this->drawWorld();
		}

		this->_window->frame(); // Commit primitives
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, this->_camera->getViewMtx().data(), this->_camera->getProjMtx().data());
	}
} // namespace physics_test