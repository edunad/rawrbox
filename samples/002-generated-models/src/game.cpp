
#include <rawrbox/render/cameras/orbit.hpp>
#include <rawrbox/render/models/mesh.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/utils/debug_draw.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>
#include <rawrbox/utils/timer.hpp>

#include <model/game.hpp>

#include <cmath>
#include <cstdio>
#include <vector>

namespace model {
	void Game::setupGLFW() {
#if defined(_DEBUG) && defined(RAWRBOX_SUPPORT_DX12)
		auto* window = rawrbox::Window::createWindow(Diligent::RENDER_DEVICE_TYPE_D3D12); // DX12 is faster on DEBUG than Vulkan, due to vulkan having extra check steps to prevent you from doing bad things
#else
		auto* window = rawrbox::Window::createWindow();
#endif
		window->setMonitor(-1);
		window->setTitle("GENERATED MODEL TEST");
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
		auto* cam = render->createCamera<rawrbox::CameraOrbit>(*window);
		cam->setPos({0.F, 6.F, -6.F});
		cam->setAngle({0.F, rawrbox::MathUtils::toRad(-55), 0.F, 0.F});
		cam->canUseKeyboard([]() { return true; });
		cam->canUseMouse([]() { return true; });
		// --------------

		// BINDS ----
		window->onKey += [this](rawrbox::Window& /*w*/, uint32_t key, uint32_t /*scancode*/, uint32_t action, uint32_t /*mods*/) {
			if (!this->_ready || action != rawrbox::KEY_ACTION_UP) return;
			if (key == rawrbox::KEY_F1) this->_debugDraw = !this->_debugDraw;
			if (key == rawrbox::KEY_F2) this->_stress = !this->_stress;
		};
		// -----

		// Add loaders
		rawrbox::RESOURCES::addLoader<rawrbox::TextureLoader>();
		// --------------

		render->init();
	}

	void Game::loadContent() {
		std::vector<std::pair<std::string, uint32_t>> initialContentFiles = {
		    {"./assets/textures/displacement.png", 0},
		    {"./assets/textures/displacement.vertex.png", 0},
		    {"./assets/textures/screem.png", 0},
		    {"./assets/textures/meow3.gif", 0},
		    {"./assets/textures/fire1.gif", 0},
		    {"./assets/textures/UV.png", 0},
		    {"./assets/textures/spline_tex.png", 0},
		};

		rawrbox::RESOURCES::loadListAsync(initialContentFiles, [this]() {
			rawrbox::runOnRenderThread([this]() {
				this->contentLoaded();
			});
		});
	}

	void Game::createModels() {
		auto* texture = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/meow3.gif")->get();
		auto* texture2 = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/screem.png")->get();
		auto* texture3 = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/UV.png")->get();

		// GRID ----
		this->_model->addMesh(rawrbox::MeshUtils::generateGrid(12, {0.F, 0.F, 0.F}));
		// --------

		// CUBE ----
		{
			auto mesh = rawrbox::MeshUtils::generateCube({3.5F, 0, 2.5F}, {1.0F, 1.0F, 1.0F}, rawrbox::Colors::White());
			this->_model->addMesh(mesh);
			this->_bboxes.push_back({{3.5F, 0, 2.5F}, mesh.getBBOX()});
		}

		{
			auto mesh = rawrbox::MeshUtils::generateCube({1.5F, 0, 2.5F}, {.5F, .5F, .5F}, rawrbox::Colors::White());
			this->_model->addMesh(mesh);
			this->_bboxes.push_back({{1.5F, 0, 2.5F}, mesh.getBBOX()});
		}

		{
			auto mesh = rawrbox::MeshUtils::generateCube({-2, 0, 0}, {0.5F, 0.5F, 0.5F}, rawrbox::Colors::White());
			mesh.setTexture(texture2);

			this->_model->addMesh(mesh);
			this->_bboxes.push_back({{-2, 0, 0}, mesh.getBBOX()});
		}
		// --------

		// PLANE -----
		{
			auto mesh = rawrbox::MeshUtils::generatePlane({2, 0, 0}, {0.5F, 0.5F});
			mesh.setTexture(texture);

			this->_bboxes.push_back({{2, 0, 0}, mesh.getBBOX()});
			this->_model->addMesh(mesh);
		}
		// ----------

		// TRIANGLE -----
		{
			rawrbox::Vector3f pos = {3.25F, -0.25F, 0};
			rawrbox::Vector3f size = {0.5F, 0.5F, 0.F};

			auto mesh = rawrbox::MeshUtils::generateTriangle(pos, rawrbox::Vector3f{0, 0, 0}, {0, 0}, rawrbox::Vector3f{size.x, size.y, 0}, {1, 0}, rawrbox::Vector3f{0, size.y, 0}, {0, 1});
			this->_bboxes.push_back({pos, mesh.getBBOX()});
			this->_model->addMesh(mesh);
		}
		// ----------

		// VERTEX SNAP ------
		{
			auto mesh = rawrbox::MeshUtils::generateCube({-3, 0, 0}, {0.5F, 0.5F, 0.5F}, rawrbox::Colors::White());
			mesh.setTexture(texture);
			mesh.setVertexSnap(24.F);

			this->_bboxes.push_back({{-3, 0, 0}, mesh.getBBOX()});
			this->_model->addMesh(mesh);
		}
		// ----------

		// ARROW ------
		{
			auto mesh = rawrbox::MeshUtils::generateArrow(0.5F, {-4.F, 0.F, 0.F}, rawrbox::Colors::White());

			this->_bboxes.push_back({{-4.F, 0.F, 0.F}, mesh.getBBOX()});
			this->_model->addMesh(mesh);
		}
		// ----

		// Sphere
		{
			auto mesh = rawrbox::MeshUtils::generateSphere({2.F, 0.F, -2.F}, {0.5F, 0.5F, 0.5F}, 0.25F);
			mesh.setTexture(texture3);

			this->_bboxes.push_back({{2.F, 0.F, -2.F}, mesh.getBBOX()});
			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateSphere({3.5F, 0.F, -2.F}, {0.5F, 0.5F, 0.5F}, 0.5F);
			mesh.setTexture(texture3);

			this->_bboxes.push_back({{3.5F, 0.F, -2.F}, mesh.getBBOX()});
			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateSphere({5.F, 0.F, -2.F}, {0.5F, 0.5F, 0.5F}, 1.F);
			mesh.setTexture(texture3);

			this->_bboxes.push_back({{5.F, 0.F, -2.F}, mesh.getBBOX()});
			this->_model->addMesh(mesh);
		}
		// -----

		// CYLINDER ------
		{
			auto mesh = rawrbox::MeshUtils::generateCylinder({-2.F, 0.F, -2.F}, {0.5F, 0.5F, 0.5F}, 12);
			mesh.setTexture(texture3);

			this->_bboxes.push_back({{-2.F, 0.F, -2.F}, mesh.getBBOX()});
			this->_model->addMesh(mesh);
		}

		// CONE ------
		{
			auto mesh = rawrbox::MeshUtils::generateCone({-3.5F, 0.F, -2.F}, {0.5F, 1.F, 0.5F}, 12);
			mesh.setTexture(texture3);

			this->_bboxes.push_back({{-3.5F, 0.F, -2.F}, mesh.getBBOX()});
			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateCone({-5.F, 0.F, -2.F}, {0.5F, 1.F, 0.5F}, 3);
			mesh.setTexture(texture3);

			this->_bboxes.push_back({{-5.F, 0.F, -2.F}, mesh.getBBOX()});
			this->_model->addMesh(mesh);
		}

		this->_model->upload();
	}

	void Game::createSpline() {
		auto* texture4 = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/spline_tex.png")->get();

		// Curve example
		rawrbox::Mesh2DShape shape;
		shape.vertex = {
		    {-0.15F, 0.025F},
		    {-0.1F, 0.025F},
		    {-0.1F, 0},

		    {0.1F, 0},
		    {0.1F, 0.025F},
		    {0.15F, 0.025F},
		};

		shape.normal = {{0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}};
		shape.u = {
		    0,
		    0.08F,
		    0.09375,

		    0.9140625F,

		    0.92F,
		    1.F};

		this->_spline->setExtrudeVerts(shape);
		this->_spline->setTexture(texture4);

		this->_spline->addPoint({0, 0, 0, 0.F}, {-1.F, 0.F, 1.F, 90.F});
		this->_spline->addPoint({-1.F, 0.F, 1.F, 90.F}, {-2.F, 0.F, 1.F, 90.F}, 0.1F);
		this->_spline->addPoint({-2.F, 0.F, 1.F, 90.F}, {-3.F, 0.F, 0.F, -180.F});
		this->_spline->addPoint({-3.F, 0.F, 0.F, -180.F}, {-2.F, 0.F, -1.F, -90.F});
		this->_spline->addPoint({-2.F, 0.F, -1.F, -90.F}, {-1.F, 0.F, -1.F, -90.F}, 0.1F);
		this->_spline->addPoint({-1.F, 0.F, -1.F, -90.F}, {0, 0.F, 0, 0.F});

		this->_spline->generateMesh();

		this->_spline->setPos({0, 0, 2.F});
		this->_spline->upload();
	}

	void Game::createDisplacement() {
		auto* textureDisplacement = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/displacement.vertex.png")->get();
		auto* texture = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/displacement.png")->get();

		auto mesh = rawrbox::MeshUtils::generateMesh({0, 0, -1.0F}, {2, 2}, 64, rawrbox::Colors::White());
		mesh.setTexture(texture);
		mesh.setDisplacementTexture(textureDisplacement, 0.5F);

		this->_displacement->addMesh(mesh);
		this->_displacement->upload();
	}

	void Game::createSprite() {
		auto* texture = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/fire1.gif")->get();

		{
			auto mesh = rawrbox::MeshUtils::generatePlane({0, 0, 0}, {0.5F, 0.5F});
			mesh.setTexture(texture);
			mesh.setBillboard(rawrbox::MeshBilldboard::Y);

			this->_sprite->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generatePlane({0, 0, 0}, {0.5F, 0.5F});
			mesh.setTexture(texture);
			mesh.setBillboard(rawrbox::MeshBilldboard::X);

			this->_sprite_2->addMesh(mesh);
		}

		this->_sprite->setPos({-0.5F, 0.25F, 0});
		this->_sprite->upload();

		this->_sprite_2->setPos({0.5F, 0.25F, 0});
		this->_sprite_2->upload();
	}

	void Game::createText() {
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "PLANE", {2.F, 0.5F, 0});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "TRIANGLE", {3.5F, 0.5F, 0});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "CUBE", {-2.F, 0.55F, 0});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "CUBE\n+ VERTEX SNAP", {-3.F, 0.55F, 0});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "SPRITE\nX AXIS", {0.5F, 0.7F, 0});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "SPRITE\nY AXIS", {-0.5F, 0.7F, 0});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "DISPLACEMENT", {0.F, 0.75F, -2});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "SPHERES", {3.5F, 0.55F, -2.F});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "CYLINDER", {-2.F, 0.55F, -2});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "CONE", {-3.5F, 0.55F, -2});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "PYRAMID", {-5.0F, 0.55F, -2});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "ARROW", {-4.0F, 0.55F, 0.F});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "SPLINE", {-1.5F, 0.55F, 2});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "1 UNIT", {3.5F, 1.0F, 2.5F});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "HALF UNIT", {1.5F, 0.55F, 2.5F});
		this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "DYNAMIC RESIZE", {0.F, 0.55F, -4.0F});
		this->_text->upload();
	}

	void Game::createDynamic() {
		this->_modelDynamic->setPos({0, 0, -4.F});
		this->_modelDynamic->upload(rawrbox::UploadType::RESIZABLE_DYNAMIC);

		float y = 0;
		rawrbox::TIMER::create(5, 700, [this, y]() mutable {
			this->_modelDynamic->addMesh(rawrbox::MeshUtils::generateCube({y - 2.F, 0.F, .0F}, {0.5F, 0.5F, 0.5F}, rawrbox::Colors::White()));
			this->_modelDynamic->updateBuffers();

			y += 1.F;
		});
	}

	void Game::contentLoaded() {
		if (this->_ready) return;

		// Model test ----
		this->createModels();
		// ----

		// Displacement test ----
		this->createDisplacement();
		// -----

		// Sprite test ----
		this->createSprite();
		// -----

		// Spline test ----
		this->createSpline();
		// -----

		// Text test ----
		this->createText();
		// ------

		// Text dynamic ----
		this->createDynamic();
		// ------

		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_RENDER) {
			this->_model.reset();
			this->_displacement.reset();
			this->_sprite.reset();
			this->_sprite_2.reset();
			this->_spline.reset();
			this->_text.reset();

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

		if (this->_model->isUploaded()) this->_model->draw();
		if (this->_modelDynamic->isUploaded()) this->_modelDynamic->draw();
		if (this->_displacement->isUploaded()) this->_displacement->draw();
		if (this->_sprite->isUploaded()) this->_sprite->draw();
		if (this->_sprite_2->isUploaded()) this->_sprite_2->draw();
		if (this->_spline->isUploaded()) this->_spline->draw();
		if (this->_text->isUploaded()) this->_text->draw();

		// DEBUG DRAW ----
		const auto debugStart = std::chrono::steady_clock::now();

		if (this->_debugDraw) {
			// BBOX -----------------------
			for (const auto& [pos, bbox] : this->_bboxes) {
				rawrbox::DebugDraw::bbox(pos, bbox, rawrbox::Colors::Red());
			}
			// -------------------

			// AXIS GIZMO ----
			rawrbox::DebugDraw::line({0, 0, 0}, {1, 0, 0}, rawrbox::Colors::Red(), false);
			rawrbox::DebugDraw::line({0, 0, 0}, {0, 1, 0}, rawrbox::Colors::Green(), false);
			rawrbox::DebugDraw::line({0, 0, 0}, {0, 0, 1}, rawrbox::Colors::Blue(), false);
			// ---------------

			// BOUNDS --------
			rawrbox::DebugDraw::aabb({1.5F, -0.5F, -3.F}, {6.F, 1.F, -1.F}, rawrbox::Colors::Yellow());
			// ---------------

			// RANDOM SHAPES -
			rawrbox::DebugDraw::quad({-1.5F, 0.F, -5.5F}, {1.5F, 0.F, -5.5F}, {1.5F, 0.F, -4.5F}, {-1.5F, 0.F, -4.5F}, rawrbox::Colors::Purple());
			rawrbox::DebugDraw::triangle({5.F, 0.F, 0.F}, {6.F, 0.F, 0.F}, {5.5F, 1.F, 0.F}, rawrbox::Colors::Orange());
			// -----------------------
		}

		if (this->_stress) this->drawDebugStress();
		rawrbox::DebugDraw::draw(); // Flush everything queued this frame

		// Measure CPU cost of queueing + submitting ---
		this->_debugTimeAccum += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - debugStart).count();
		if (++this->_debugFrames >= 120) {
			this->_debugAvgMs = this->_debugTimeAccum / this->_debugFrames;
			if (this->_stress) {
				fmt::print("[DebugDraw stress] avg {:.3f} ms / frame\n", this->_debugAvgMs);
				std::fflush(stdout);
			}

			this->_debugTimeAccum = 0.0;
			this->_debugFrames = 0;
		}
		// ---------------------------------------------
	}

	void Game::drawDebugStress() {
		const float time = std::chrono::duration<float>(std::chrono::steady_clock::now() - this->_startTime).count();

		// STATIC: 10k lines in 256 colors (100 rings x 100 segments) ----
		for (int ring = 0; ring < 100; ring++) {
			const float y = 1.5F + static_cast<float>(ring) * 0.02F;
			const float r = 5.F + std::sin(static_cast<float>(ring) * 0.2F) * 0.5F;

			for (int seg = 0; seg < 100; seg++) {
				const float a0 = static_cast<float>(seg) / 100.F * 6.2831853F;
				const float a1 = static_cast<float>(seg + 1) / 100.F * 6.2831853F;

				const int c = (ring * 100 + seg) % 256;
				const rawrbox::Colorf color = {static_cast<float>(c % 16) / 15.F, static_cast<float>(c / 16) / 15.F, 1.F - (static_cast<float>(c % 16) / 15.F), 1.F};

				rawrbox::DebugDraw::line({std::cos(a0) * r, y, std::sin(a0) * r}, {std::cos(a1) * r, y, std::sin(a1) * r}, color);
			}
		}

		// STATIC: 1k triangles in 64 colors ----
		for (int i = 0; i < 1000; i++) {
			const float x = static_cast<float>(i % 40) * 0.3F - 6.F;
			const float z = static_cast<float>(i / 40) * 0.3F - 9.F;

			const int c = i % 64;
			const rawrbox::Colorf color = {static_cast<float>(c % 8) / 7.F, static_cast<float>(c / 8) / 7.F, 0.5F, 1.F};

			rawrbox::DebugDraw::triangle({x, 0.01F, z}, {x + 0.25F, 0.01F, z}, {x + 0.125F, 0.01F, z + 0.25F}, color);
		}

		// DYNAMIC: 2k spinning lines in 8 colors ----
		for (int i = 0; i < 2000; i++) {
			const float a = (static_cast<float>(i) / 2000.F * 6.2831853F) + time;
			const float len = 1.F + (static_cast<float>(i % 10) * 0.1F);

			const int c = i % 8;
			const rawrbox::Colorf color = {(c & 1) != 0 ? 1.F : 0.2F, (c & 2) != 0 ? 1.F : 0.2F, (c & 4) != 0 ? 1.F : 0.2F, 1.F};

			rawrbox::DebugDraw::line({0, 3.5F, 0}, {std::cos(a) * len, 3.5F + (std::sin(a * 3.F) * 0.25F), std::sin(a) * len}, color);
		}
	}

	void Game::drawOverlay() const {
		if (!this->_ready) return;
		auto* stencil = rawrbox::RENDERER->stencil();
		stencil->drawText(fmt::format("[F1]   DEBUG DRAW -> {}", this->_debugDraw ? "enabled" : "disabled"), {15, 15}, rawrbox::Colors::White(), rawrbox::Colors::Black());

		if (this->_debugDraw) {
			const auto& stats = rawrbox::DebugDraw::stats();
			stencil->drawText(fmt::format("      buckets: {} | points: {} | uploads: {} | draw calls: {} | cpu: {:.3f} ms", stats.buckets, stats.points, stats.uploads, stats.drawCalls, this->_debugAvgMs), {15, 28}, rawrbox::Colors::White(), rawrbox::Colors::Black());
		}

		stencil->drawText(fmt::format("[F2]   DEBUG DRAW STRESS -> {}", this->_stress ? "enabled" : "disabled"), {15, 41}, rawrbox::Colors::White(), rawrbox::Colors::Black());
	}

	void Game::draw() {
		rawrbox::Window::render(); // Draw world, overlay & commit primitives
	}

} // namespace model
