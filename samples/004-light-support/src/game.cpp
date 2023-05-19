

#include <rawrbox/render/model/assimp/assimp_importer.hpp>
#include <rawrbox/render/resources/assimp/model.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>

#include <light/game.hpp>

#include <bx/bx.h>
#include <bx/math.h>

#include <vector>

namespace light {
	void Game::init() {
		int width = 1024;
		int height = 768;

		this->_window = std::make_shared<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("LIGHT TEST");
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

		rawrbox::RESOURCES::addLoader(std::make_unique<rawrbox::FontLoader>());
		rawrbox::RESOURCES::addLoader(std::make_unique<rawrbox::AssimpLoader>());

		// Load content ---
		this->loadContent();
		// -----
	}

	void Game::loadContent() {
		std::array<std::pair<std::string, uint32_t>, 2> initialContentFiles = {
		    std::make_pair<std::string, uint32_t>("cour.ttf", 0),
		    std::make_pair<std::string, uint32_t>("content/models/light_test/test.fbx", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_LIGHT | rawrbox::ModelLoadFlags::Debug::PRINT_MATERIALS)};

		rawrbox::ASYNC::run([initialContentFiles]() {
			for (auto& f : initialContentFiles) {
				rawrbox::RESOURCES::loadFile(f.first, f.second);
			} }, [this] { rawrbox::runOnMainThread([this]() {
										  rawrbox::RESOURCES::upload();
										  this->contentLoaded();
									  }); });

		this->_window->upload();
	}

	void Game::contentLoaded() {
		this->_font = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>("cour.ttf")->getSize(16);

		// Assimp test ---
		auto mdl = rawrbox::RESOURCES::getFile<rawrbox::ResourceAssimp>("./content/models/light_test/test.fbx");

		this->_model->load(mdl->model);
		this->_model->setPos({0, 0, 0});
		//   -----

		// Text test ----
		{
			this->_text->addText(this->_font, "SPOT LIGHT", {-6.F, 1.8F, 0});
			this->_text->addText(this->_font, "DIRECTIONAL LIGHT", {0.F, 1.8F, 0});
			this->_text->addText(this->_font, "POINT LIGHT", {6.F, 1.8F, 0});
			this->_text->upload();
		}
		// ------

		this->_ready = true;
	}

	void Game::shutdown() {
		this->_window = nullptr;
		this->_camera = nullptr;

		this->_model = nullptr;
		this->_text = nullptr;

		rawrbox::RESOURCES::shutdown();
		rawrbox::LIGHTS::shutdown();
		rawrbox::Engine::shutdown();
	}

	void Game::pollEvents() {
		if (this->_window == nullptr) return;
		this->_window->pollEvents();
	}

	float t = 0.F;
	void Game::update() {
		if (this->_camera == nullptr) return;
		this->_camera->update();

		t += 0.45F;
		this->_model->setAngle({0, bx::toRad(t), 0});
	}

	void Game::drawWorld() {
		if (this->_model == nullptr || this->_text == nullptr) return;

		this->_model->draw(this->_camera->getPos());
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
		bgfx::dbgTextPrintf(1, 1, 0x1f, "004-light-support");
		bgfx::dbgTextPrintf(1, 2, 0x3f, "Description: Light test");
		printFrames();
		// -----------

		if (this->_ready) {
			this->drawWorld();
		} else {
			bgfx::dbgTextPrintf(1, 10, 0x70, "                                   ");
			bgfx::dbgTextPrintf(1, 11, 0x70, "          LOADING CONTENT          ");
			bgfx::dbgTextPrintf(1, 12, 0x70, "                                   ");
		}

		this->_window->frame(true); // Commit primitives
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, this->_camera->getViewMtx().data(), this->_camera->getProjMtx().data());
	}
} // namespace light
