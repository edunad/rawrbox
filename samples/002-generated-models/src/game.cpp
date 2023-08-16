
#include <rawrbox/render/camera/orbital.hpp>
#include <rawrbox/render/model/mesh.hpp>
#include <rawrbox/render/model/utils/mesh.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>

#include <model/game.hpp>

#include <bx/bx.h>
#include <bx/math.h>

#include <vector>

namespace model {

	void Game::setupGLFW() {
		this->_window = std::make_unique<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("GENERATED MODEL TEST");
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
		cam->setAngle({0.F, bx::toRad(-45), 0.F, 0.F});
		// --------------

		rawrbox::RESOURCES::addLoader<rawrbox::TextureLoader>();
		rawrbox::RESOURCES::addLoader<rawrbox::FontLoader>();

		// Load content ---
		this->loadContent();
		// -----
	}

	void Game::loadContent() {

		std::array initialContentFiles = {
		    std::make_pair<std::string, uint32_t>("cour.ttf", 0),
		    std::make_pair<std::string, uint32_t>("content/textures/screem.png", 0),
		    std::make_pair<std::string, uint32_t>("content/textures/meow3.gif", 0),
		    std::make_pair<std::string, uint32_t>("content/textures/displacement_test.png", 0),
		    std::make_pair<std::string, uint32_t>("content/textures/spline_tex.png", 0),
		};

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
		this->_ready = true;
		this->_font = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>("cour.ttf")->getSize(24);

		auto texture = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./content/textures/meow3.gif")->get();
		auto texture2 = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./content/textures/screem.png")->get();
		auto texture3 = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./content/textures/displacement_test.png")->get();
		auto texture4 = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./content/textures/spline_tex.png")->get();

		// Model test ----
		{
			auto mesh = rawrbox::MeshUtils::generatePlane({2, 0, 0}, {0.5F, 0.5F});
			mesh.setTexture(texture);

			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({2, 0, 0}, mesh.getBBOX()));
			this->_model->addMesh(mesh);
		}

		{
			rawrbox::Vector3f pos = {3.25F, -0.25F, 0};
			rawrbox::Vector3f size = {0.5F, 0.5F, 0.F};

			auto mesh = rawrbox::MeshUtils::generateTriangle(pos, rawrbox::Vector3f{0, 0, 0}, {0, 0}, rawrbox::Colors::Red(), rawrbox::Vector3f{size.x, size.y, 0}, {1, 0}, rawrbox::Colors::Green(), rawrbox::Vector3f{0, size.y, 0}, {0, 1}, rawrbox::Colors::Blue());
			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX(pos + size / 2, mesh.getBBOX()));
			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateCube({-2, 0, 0}, {0.5F, 0.5F, 0.5F}, rawrbox::Colors::White());
			mesh.setTexture(texture);

			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({-2, 0, 0}, mesh.getBBOX()));
			this->_model->addMesh(mesh);
		}

		{
			this->_model->addMesh(rawrbox::MeshUtils::generateCube({3.5F, 0, 2.5F}, {1.0F, 1.0F, 1.0F}, rawrbox::Colors::White()));
			this->_model->addMesh(rawrbox::MeshUtils::generateCube({1.5F, 0, 2.5F}, {.5F, .5F, .5F}, rawrbox::Colors::White()));
		}

		{
			auto mesh = rawrbox::MeshUtils::generateCube({-3, 0, 0}, {0.5F, 0.5F, 0.5F}, rawrbox::Colors::White());
			mesh.setTexture(texture);
			mesh.setVertexSnap(24.F);

			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({-3, 0, 0}, mesh.getBBOX()));
			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateAxis(1, {0.F, 0.F, 0.F});

			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({0, 0, 0}, mesh.getBBOX()));
			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateGrid(12, {0.F, 0.F, 0.F});
			this->_model->addMesh(mesh);
		}

		// Sphere
		{
			auto mesh = rawrbox::MeshUtils::generateSphere({2.F, 0.F, -2.F}, 0.5F, 0);

			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({2.F, 0.F, -2.F}, mesh.getBBOX()));
			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateSphere({3.5F, 0.F, -2.F}, 0.5F, 1);

			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({3.5F, 0.F, -2.F}, mesh.getBBOX()));
			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateSphere({5.F, 0.F, -2.F}, 0.5F, 2);

			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({5.F, 0.F, -2.F}, mesh.getBBOX()));
			this->_model->addMesh(mesh);
		}
		// -----

		{
			auto mesh = rawrbox::MeshUtils::generateCylinder({-2.F, 0.F, -2.F}, {0.5F, 0.5F, 0.5F}, 12);

			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({-2.F, 0.F, -2.F}, mesh.getBBOX()));
			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateCone({-3.5F, 0.F, -2.F}, {0.5F, 1.F, 0.5F}, 12);

			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({-3.5F, 0.F, -2.F}, mesh.getBBOX()));
			this->_model->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateCone({-5.F, 0.F, -2.F}, {0.5F, 1.F, 0.5F}, 4);

			this->_bboxes->addMesh(rawrbox::MeshUtils::generateBBOX({-5.F, 0.F, -2.F}, mesh.getBBOX()));
			this->_model->addMesh(mesh);
		}

		// Displacement test ----
		{
			auto mesh = rawrbox::MeshUtils::generateMesh({0, 0, 0}, 64, rawrbox::Colors::White());
			mesh.setTexture(texture3);
			mesh.setDisplacementTexture(texture3, 24.F);

			this->_displacement->addMesh(mesh);
		}

		{
			auto mesh = rawrbox::MeshUtils::generateMesh({0, 0.5F, 0}, 64, rawrbox::Colors::Black());
			mesh.setDisplacementTexture(texture3, 24.F);
			mesh.lineMode = true;

			this->_displacement->addMesh(mesh);
		}

		this->_displacement->setPos({0, 0.1F, -2});
		this->_displacement->setScale({0.025F, 0.025F, 0.025F});
		// -----

		// Sprite test ----
		{
			auto mesh = rawrbox::MeshUtils::generateCube({0, 1, 0}, {0.2F, 0.2F});
			mesh.setTexture(texture2);
			this->_sprite->addMesh(mesh);
		}
		// -----

		// Spline test ----

		{
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
		}

		// -----

		// Text test ----
		this->_text->addText(*this->_font, "PLANE", {2.F, 0.5F, 0});
		this->_text->addText(*this->_font, "TRIANGLE", {3.5F, 0.5F, 0});
		this->_text->addText(*this->_font, "CUBE", {-2.F, 0.55F, 0});
		this->_text->addText(*this->_font, "CUBE\nVertex snap", {-3.F, 0.55F, 0});
		this->_text->addText(*this->_font, "AXIS", {0.F, 0.5F, 0});
		this->_text->addText(*this->_font, "SPRITE", {0.F, 1.2F, 0});
		this->_text->addText(*this->_font, "DISPLACEMENT", {0.F, 1.2F, -2});
		this->_text->addText(*this->_font, "SPHERES", {3.5F, 0.55F, -2.F});
		this->_text->addText(*this->_font, "CYLINDER", {-2.F, 0.55F, -2});
		this->_text->addText(*this->_font, "CONE", {-3.5F, 0.55F, -2});
		this->_text->addText(*this->_font, "PYRAMID", {-5.0F, 0.55F, -2});

		this->_text->addText(*this->_font, "SPLINE", {-1.5F, 0.55F, 2});

		this->_text->addText(*this->_font, "1 UNIT", {3.5F, 1.0F, 2.5F});
		this->_text->addText(*this->_font, "HALF UNIT", {1.5F, 0.55F, 2.5F});
		// ------

		this->_model->upload();
		this->_displacement->upload();
		this->_sprite->upload();
		this->_spline->upload();
		this->_bboxes->upload();
		this->_text->upload();
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) return;

		this->_displacement.reset();
		this->_model.reset();
		this->_sprite.reset();
		this->_bboxes.reset();
		this->_spline.reset();
		this->_text.reset();

		rawrbox::RESOURCES::shutdown();
		rawrbox::ASYNC::shutdown();

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
	}

	void Game::drawWorld() {
		if (!this->_ready) return;
		if (this->_model == nullptr || this->_sprite == nullptr || this->_text == nullptr || this->_displacement == nullptr || this->_spline == nullptr) return;

		this->_model->draw();
		this->_displacement->draw();
		this->_sprite->draw();
		this->_spline->draw();
		this->_bboxes->draw();
		this->_text->draw();
	}

	void Game::printFrames() {
		const bgfx::Stats* stats = bgfx::getStats();

		bgfx::dbgTextPrintf(1, 4, 0x6f, "GPU %0.6f [ms]", double(stats->gpuTimeEnd - stats->gpuTimeBegin) * 1000.0 / stats->gpuTimerFreq);
		bgfx::dbgTextPrintf(1, 5, 0x6f, "CPU %0.6f [ms]", double(stats->cpuTimeEnd - stats->cpuTimeBegin) * 1000.0 / stats->cpuTimerFreq);
		bgfx::dbgTextPrintf(1, 7, 0x5f, fmt::format("TRIANGLES: {}", stats->numPrims[bgfx::Topology::TriList]).c_str());
		bgfx::dbgTextPrintf(1, 8, 0x5f, fmt::format("DRAW CALLS: {}", stats->numDraw).c_str());
		bgfx::dbgTextPrintf(1, 9, 0x5f, fmt::format("COMPUTE CALLS: {}", stats->numCompute).c_str());
	}

	void Game::draw() {
		if (this->_window == nullptr) return;

		// DEBUG ----
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(1, 1, 0x1f, "002-generated-models");
		bgfx::dbgTextPrintf(1, 2, 0x3f, "Description: Generated models test");
		printFrames();
		// -----------

		if (!this->_ready) {
			bgfx::dbgTextPrintf(1, 10, 0x70, "                                   ");
			bgfx::dbgTextPrintf(1, 11, 0x70, "          LOADING CONTENT          ");
			bgfx::dbgTextPrintf(1, 12, 0x70, "                                   ");
		}

		this->_window->render(); // Commit primitives
	}

} // namespace model
