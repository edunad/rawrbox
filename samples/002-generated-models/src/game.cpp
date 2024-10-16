
#include <rawrbox/render/cameras/orbital.hpp>
#include <rawrbox/render/models/mesh.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>
#include <rawrbox/utils/timer.hpp>

#include <model/game.hpp>

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
		auto* cam = render->createCamera<rawrbox::CameraOrbital>(*window);
		cam->setPos({0.F, 6.F, -6.F});
		cam->setAngle({0.F, rawrbox::MathUtils::toRad(-55), 0.F, 0.F});
		cam->onMovementStart = []() { fmt::print("Camera start\n"); };
		cam->onMovementStop = []() { fmt::print("Camera stop\n"); };
		// --------------

		// BINDS ----
		window->onKey += [this](rawrbox::Window& /*w*/, uint32_t key, uint32_t /*scancode*/, uint32_t action, uint32_t /*mods*/) {
			if (!this->_ready || action != rawrbox::KEY_ACTION_UP || key != rawrbox::KEY_F1) return;
			this->_bbox = !this->_bbox;
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
			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({3.5F, 0, 2.5F}, mesh.getBBOX()));
		}

		{
			auto mesh = rawrbox::MeshUtils::generateCube({1.5F, 0, 2.5F}, {.5F, .5F, .5F}, rawrbox::Colors::White());
			this->_model->addMesh(mesh);
			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({1.5F, 0, 2.5F}, mesh.getBBOX()));
		}

		{
			auto mesh = rawrbox::MeshUtils::generateCube({-2, 0, 0}, {0.5F, 0.5F, 0.5F}, rawrbox::Colors::White());
			mesh.setTexture(texture2);

			this->_model->addMesh(mesh);
			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({-2, 0, 0}, mesh.getBBOX()));
		}
		// --------

		// PLANE -----
		{
			auto mesh = rawrbox::MeshUtils::generatePlane({2, 0, 0}, {0.5F, 0.5F});
			mesh.setTexture(texture);

			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({2, 0, 0}, mesh.getBBOX()));
			this->_model->addMesh(mesh);
		}
		// ----------

		// TRIANGLE -----
		{
			rawrbox::Vector3f pos = {3.25F, -0.25F, 0};
			rawrbox::Vector3f size = {0.5F, 0.5F, 0.F};

			auto mesh = rawrbox::MeshUtils::generateTriangle(pos, rawrbox::Vector3f{0, 0, 0}, {0, 0}, rawrbox::Vector3f{size.x, size.y, 0}, {1, 0}, rawrbox::Vector3f{0, size.y, 0}, {0, 1});
			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX(pos, mesh.getBBOX()));
			this->_model->addMesh(mesh);
		}
		// ----------

		// VERTEX SNAP ------
		{
			auto mesh = rawrbox::MeshUtils::generateCube({-3, 0, 0}, {0.5F, 0.5F, 0.5F}, rawrbox::Colors::White());
			mesh.setTexture(texture);
			mesh.setVertexSnap(24.F);

			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({-3, 0, 0}, mesh.getBBOX()));
			this->_model->addMesh(mesh);
		}
		// ----------

		// ARROW ------
		{
			auto mesh = rawrbox::MeshUtils::generateArrow(0.5F, {-4.F, 0.F, 0.F}, rawrbox::Colors::White());

			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({-4.F, 0.F, 0.F}, mesh.getBBOX()));
			this->_model->addMesh(mesh);
		}
		// ----

		// Sphere
		{
			auto mesh = rawrbox::MeshUtils::generateSphere({2.F, 0.F, -2.F}, {0.5F, 0.5F, 0.5F}, 0.25F);
			mesh.setTexture(texture3);

			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({2.F, 0.F, -2.F}, mesh.getBBOX()));
			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateSphere({3.5F, 0.F, -2.F}, {0.5F, 0.5F, 0.5F}, 0.5F);
			mesh.setTexture(texture3);

			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({3.5F, 0.F, -2.F}, mesh.getBBOX()));
			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateSphere({5.F, 0.F, -2.F}, {0.5F, 0.5F, 0.5F}, 1.F);
			mesh.setTexture(texture3);

			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({5.F, 0.F, -2.F}, mesh.getBBOX()));
			this->_model->addMesh(mesh);
		}
		// -----

		// CYLINDER ------
		{
			auto mesh = rawrbox::MeshUtils::generateCylinder({-2.F, 0.F, -2.F}, {0.5F, 0.5F, 0.5F}, 12);
			mesh.setTexture(texture3);

			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({-2.F, 0.F, -2.F}, mesh.getBBOX()));
			this->_model->addMesh(mesh);
		}

		// CONE ------
		{
			auto mesh = rawrbox::MeshUtils::generateCone({-3.5F, 0.F, -2.F}, {0.5F, 1.F, 0.5F}, 12);
			mesh.setTexture(texture3);

			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({-3.5F, 0.F, -2.F}, mesh.getBBOX()));
			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateCone({-5.F, 0.F, -2.F}, {0.5F, 1.F, 0.5F}, 3);
			mesh.setTexture(texture3);

			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({-5.F, 0.F, -2.F}, mesh.getBBOX()));
			this->_model->addMesh(mesh);
		}

		this->_model->upload();
		this->_bboxes->upload();
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
			this->_bboxes.reset();
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
		if (this->_bboxes->isUploaded() && this->_bbox) this->_bboxes->draw();
		if (this->_text->isUploaded()) this->_text->draw();
	}

	void Game::drawOverlay() const {
		if (!this->_ready) return;
		auto* stencil = rawrbox::RENDERER->stencil();

		stencil->drawText(fmt::format("[F1]   BBOX -> {}", this->_bbox ? "enabled" : "disabled"), {15, 15}, rawrbox::Colors::White(), rawrbox::Colors::Black());
	}

	void Game::draw() {
		rawrbox::Window::render(); // Draw world, overlay & commit primitives
	}

} // namespace model
