
#include <rawrbox/render/model/mesh.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/render/resources/gif.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/resources/static.hpp>
#include <rawrbox/utils/keys.hpp>

#include <model/game.hpp>

#include <bx/bx.h>
#include <bx/math.h>

#include <vector>

namespace model {
	void Game::init() {
		int width = 1024;
		int height = 768;

		this->_window = std::make_shared<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("SIMPLE MODEL TEST");
		this->_window->setRenderer(bgfx::RendererType::Count);
		this->_window->onWindowClose += [this](auto& w) {
			this->shutdown();
		};

		this->_window->initialize(width, height, rawrbox::WindowFlags::Debug::TEXT | rawrbox::WindowFlags::Debug::PROFILER | rawrbox::WindowFlags::Window::WINDOWED);

		// Setup camera
		this->_camera = std::make_shared<rawrbox::CameraOrbital>(this->_window);
		this->_camera->setPos({0.F, 5.F, -5.F});
		this->_camera->setAngle({0.F, bx::toRad(-45), 0.F, 0.F});
		// --------------

		rawrbox::Resources.addLoader(std::make_unique<rawrbox::TextureLoader>());
		rawrbox::Resources.addLoader(std::make_unique<rawrbox::GIFLoader>());
		rawrbox::Resources.addLoader(std::make_unique<rawrbox::FontLoader>());

		// Load content ---
		this->loadContent();
		// -----
	}

	void Game::loadContent() {
		std::array<std::string, 3> initialContentFiles = {
		    "cour.ttf",
		    "content/textures/screem.png",
		    "content/textures/meow3.gif",
		};

		rawrbox::ASYNC.run([initialContentFiles]() {
			for (auto& f : initialContentFiles) {
				rawrbox::Resources.loadFile( f);
			} },
		    [this] {
			    rawrbox::runOnMainThread([this]() {
				    rawrbox::Resources.upload();
				    this->contentLoaded();
			    });
		    });

		this->_window->upload();
	}

	void Game::contentLoaded() {
		this->_ready = true;

		this->_texture = rawrbox::Resources.getFile<rawrbox::ResourceTexture>("./content/textures/screem.png")->texture;
		this->_texture2 = rawrbox::Resources.getFile<rawrbox::ResourceGIF>("./content/textures/meow3.gif")->texture;

		this->_font = rawrbox::Resources.getFile<rawrbox::ResourceFont>("cour.ttf")->getSize(16);

		// Model test ----
		{
			auto mesh = this->_model->generatePlane({2, 0, 0}, {0.5F, 0.5F});
			mesh->setTexture(this->_texture);
			this->_model->addMesh(mesh);
		}

		{
			auto mesh = this->_model->generateCube({-2, 0, 0}, {0.5F, 0.5F, 0.5F}, rawrbox::Colors::White);
			mesh->setTexture(this->_texture2);
			this->_model->addMesh(mesh);
		}

		{
			auto mesh = this->_model->generateAxis(1, {0.F, 0.F, 0.F});
			this->_model->addMesh(mesh);
		}

		{
			auto mesh = this->_model->generateGrid(12, {0.F, 0.F, 0.F});
			this->_model->addMesh(mesh);
		}

		// -----

		// Sprite test ----
		{
			auto mesh = this->_sprite->generatePlane({0, 1, 0}, {0.2F, 0.2F});
			mesh->setTexture(this->_texture);
			this->_sprite->addMesh(mesh);
		}
		// -----
		// Text test ----
		{
			this->_text->addText(this->_font, "PLANE", {2.F, 0.5F, 0});
			this->_text->addText(this->_font, "CUBE", {-2.F, 0.55F, 0});
			this->_text->addText(this->_font, "AXIS", {0.F, 0.5F, 0});
			this->_text->addText(this->_font, "SPRITE", {0.F, 1.2F, 0});
		}
		// ------

		this->_model->upload();
		this->_sprite->upload();
		this->_text->upload();
	}

	void Game::shutdown() {
		this->_window = nullptr;
		this->_camera = nullptr;
		this->_texture2 = nullptr;

		this->_model = nullptr;
		this->_sprite = nullptr;
		this->_text = nullptr;

		rawrbox::Engine::shutdown();
	}

	void Game::pollEvents() {
		if (this->_window == nullptr) return;
		this->_window->pollEvents();
	}

	void Game::update() {
		if (this->_texture2 == nullptr || this->_camera == nullptr) return;
		this->_camera->update();
		this->_texture2->step();
	}

	void Game::drawWorld() {
		if (this->_model == nullptr || this->_sprite == nullptr || this->_text == nullptr) return;

		this->_model->draw(this->_camera->getPos());
		this->_sprite->draw(this->_camera->getPos());
		this->_text->draw(this->_camera->getPos());
	}

	void printFrames() {
		const bgfx::Stats* stats = bgfx::getStats();

		bgfx::dbgTextPrintf(1, 4, 0x6f, "GPU %0.6f [ms]", double(stats->gpuTimeEnd - stats->gpuTimeBegin) * 1000.0 / stats->gpuTimerFreq);
		bgfx::dbgTextPrintf(1, 5, 0x6f, "CPU %0.6f [ms]", double(stats->cpuTimeEnd - stats->cpuTimeBegin) * 1000.0 / stats->cpuTimerFreq);
		bgfx::dbgTextPrintf(1, 6, 0x6f, fmt::format("TRIANGLES: {} ----->    DRAW CALLS: {}", stats->numPrims[bgfx::Topology::TriList], stats->numDraw).c_str());
	}

	void Game::draw() {
		if (this->_window == nullptr) return;
		this->_window->clear(); // Clean up and set renderer

		// DEBUG ----
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(1, 1, 0x1f, "002-generated-models");
		bgfx::dbgTextPrintf(1, 2, 0x3f, "Description: Generated models test");
		printFrames();
		// -----------

		if (this->_ready) {
			this->drawWorld();
		} else {
			bgfx::dbgTextPrintf(1, 10, 0x70, "                                   ");
			bgfx::dbgTextPrintf(1, 11, 0x70, "          LOADING CONTENT          ");
			bgfx::dbgTextPrintf(1, 12, 0x70, "                                   ");
		}

		this->_window->frame(); // Commit primitives
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, this->_camera->getViewMtx().data(), this->_camera->getProjMtx().data());
	}

} // namespace model
