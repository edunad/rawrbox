
#include <rawrbox/physics_2d/manager.hpp>
#include <rawrbox/physics_2d/utils.hpp>
#include <rawrbox/render/camera/orbital.hpp>
#include <rawrbox/render/model/utils/mesh.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>

#include <phys_2d_test/game.hpp>

#include <fmt/format.h>

namespace phys_2d_test {
	void Game::setupGLFW() {
		this->_window = std::make_unique<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("2D PHYSICS TEST");
		this->_window->setRenderer<>(
		    bgfx::RendererType::Count, []() {}, [this]() { this->drawWorld(); });
		this->_window->create(1024, 768, rawrbox::WindowFlags::Debug::TEXT | rawrbox::WindowFlags::Debug::PROFILER | rawrbox::WindowFlags::Window::WINDOWED | rawrbox::WindowFlags::Features::MULTI_THREADED);
		this->_window->onWindowClose += [this](auto& /*w*/) { this->shutdown(); };
	}

	void Game::init() {
		if (this->_window == nullptr) return;
		this->_window->initializeBGFX();

		// Setup camera
		auto cam = this->_window->setupCamera<rawrbox::CameraOrbital>(*this->_window);
		cam->setPos({0.F, 5.F, -5.F});
		cam->setAngle({0.F, 0.F, 0.F, 0.F});
		// --------------

		// Setup loaders
		rawrbox::RESOURCES::addLoader<rawrbox::TextureLoader>();
		// ----------

		// Setup physics
		rawrbox::PHYSICS_2D::physSettings->sleeping_treshold = 0.8F;
		rawrbox::PHYSICS_2D::physSettings->apply_gravity = false;
		rawrbox::PHYSICS_2D::init();
		/*rawrbox::PHYSICS_2D::onContact += [](const muli::ContactManifold& contact) {
			fmt::print("[RawrBox-Physics2D] Body {} contacted with body {}\n", contact.contactPoints[0].id, contact.contactPoints[1].id);
		};*/
		// ----

		// Load content ---
		this->loadContent();
		//   -----
	}

	void Game::loadContent() {
		std::array initialContentFiles = {
		    std::make_pair<std::string, uint32_t>("./content/textures/crate_hl1.png", 0)};

		this->_loadingFiles = static_cast<int>(initialContentFiles.size());
		for (auto& f : initialContentFiles) {
			rawrbox::RESOURCES::loadFileAsync(f.first, f.second, [this]() {
				this->_loadingFiles--;
				if (this->_loadingFiles <= 0) {
					rawrbox::runOnRenderThread([this]() { this->contentLoaded(); });
				}
			});
		}

		this->_window->upload();
	}

	void Game::contentLoaded() {
		// Setup grid --
		{
			auto mesh = rawrbox::MeshUtils::generateGrid(12, {0.F, 0.F, 0.F});
			this->_modelGrid->addMesh(mesh);
		}

		this->_modelGrid->setPos({0, 5.F, 0.1F});
		this->_modelGrid->setEulerAngle({bx::toRad(90), 0, 0});
		this->_modelGrid->upload();
		// ----

		this->_texture = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./content/textures/crate_hl1.png")->get();

		// TIMER ---
		this->_timer = rawrbox::Timer::create(
		    600, 25, [this]() { this->createBox({0, 5, 0}, {0.5F, 0.5F}); });
		this->_timer->pause(this->_paused);
		// --------

		// BINDS ----
		this->_window->onMouseKey += [this](auto& /*w*/, const rawrbox::Vector2i& /*mousePos*/, int button, int action, int /*mods*/) {
			const bool isDown = action == 1;
			if (!isDown || button != MOUSE_BUTTON_1) return;

			this->_paused = !this->_paused;
			if (this->_timer != nullptr) this->_timer->pause(this->_paused);
		};
		// -----

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
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) return;

		rawrbox::RESOURCES::shutdown();
		rawrbox::ASYNC::shutdown();
		rawrbox::PHYSICS_2D::shutdown();

		this->_texture = nullptr;
		this->_timer = nullptr;

		this->_modelGrid.reset();
		this->_boxes.clear();

		this->_window->unblockPoll();
		this->_window.reset();
	}

	void Game::pollEvents() {
		if (this->_window == nullptr) return;
		this->_window->pollEvents();
	}

	void Game::update() {
		if (this->_window == nullptr) return;
		this->_window->update();

		rawrbox::Timer::update();
	}

	void Game::fixedUpdate() {
		if (this->_paused) return;
		rawrbox::PHYSICS_2D::tick();
	}

	void Game::printFrames() {
		const bgfx::Stats* stats = bgfx::getStats();

		bgfx::dbgTextPrintf(1, 4, 0x6f, "GPU %0.6f [ms]", double(stats->gpuTimeEnd - stats->gpuTimeBegin) * 1000.0 / stats->gpuTimerFreq);
		bgfx::dbgTextPrintf(1, 5, 0x6f, "CPU %0.6f [ms]", double(stats->cpuTimeEnd - stats->cpuTimeBegin) * 1000.0 / stats->cpuTimerFreq);
		bgfx::dbgTextPrintf(1, 7, 0x5f, fmt::format("TRIANGLES: {}", stats->numPrims[bgfx::Topology::TriList]).c_str());
		bgfx::dbgTextPrintf(1, 8, 0x5f, fmt::format("DRAW CALLS: {}", stats->numDraw).c_str());
		bgfx::dbgTextPrintf(1, 9, 0x5f, fmt::format("COMPUTE CALLS: {}", stats->numCompute).c_str());

		bgfx::dbgTextPrintf(1, 11, 0x5f, fmt::format("TOTAL BODIES: {}", rawrbox::PHYSICS_2D::physWorld->GetBodyCount()).c_str());
		bgfx::dbgTextPrintf(1, 12, 0x5f, fmt::format("ACTIVE: {}", rawrbox::PHYSICS_2D::physWorld->GetBodyCount() - rawrbox::PHYSICS_2D::physWorld->GetSleepingBodyCount()).c_str());

		bgfx::dbgTextPrintf(1, 13, 0x4f, fmt::format("Mouse1 to {} simulation", this->_paused ? "unpause" : "pause").c_str());
	}

	void Game::drawWorld() {
		if (!this->_ready) return;
		if (this->_modelGrid != nullptr) this->_modelGrid->draw();

		for (auto& b : this->_boxes) {
			auto body = b->body;
			if (body == nullptr) continue;

			auto pos = rawrbox::Phys2DUtils::posToVec(body->GetPosition());

			b->mdl->setPos(pos);
			b->mdl->setEulerAngle({0, 0, body->GetAngle()});
			b->mdl->setColor(body->IsSleeping() ? rawrbox::Colors::DarkGray() : rawrbox::Colors::White());

			b->mdl->draw();
		}
	}

	void Game::draw() {
		if (this->_window == nullptr) return;

		// DEBUG ----
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(1, 1, 0x1f, "013-2d-physics");
		bgfx::dbgTextPrintf(1, 2, 0x3f, "Description: 2D PHYSICS test");
		this->printFrames();
		// -----------

		if (!this->_ready) {
			bgfx::dbgTextPrintf(1, 10, 0x70, "                                   ");
			bgfx::dbgTextPrintf(1, 11, 0x70, "          LOADING CONTENT          ");
			bgfx::dbgTextPrintf(1, 12, 0x70, "                                   ");
		}

		this->_window->render(); // Commit primitives
	}
} // namespace phys_2d_test
