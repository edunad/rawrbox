#include <rawrbox/debug/gizmos.hpp>
#include <rawrbox/render/model/assimp/assimp_importer.hpp>
#include <rawrbox/render/resources/assimp/model.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>

#include <assimp/game.hpp>

#include <bx/bx.h>
#include <bx/math.h>

#include <vector>

namespace assimp {

	void Game::setupGLFW() {
		this->_window = std::make_shared<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("ASSIMP TEST");
		this->_window->setRenderer(bgfx::RendererType::Count);
		this->_window->create(1024, 768, rawrbox::WindowFlags::Debug::TEXT | rawrbox::WindowFlags::Debug::PROFILER | rawrbox::WindowFlags::Window::WINDOWED);
		this->_window->onWindowClose += [this](auto& w) {
			this->shutdown();
		};
	}

	void Game::init() {
		if (this->_window == nullptr) return;
		this->_window->initializeBGFX();

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
		std::array initialContentFiles = {
		    std::make_pair<std::string, uint32_t>("cour.ttf", 0),
		    std::make_pair<std::string, uint32_t>("content/models/ps1_phasmophobia/Phasmaphobia_Semi.fbx", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_LIGHT),
		    std::make_pair<std::string, uint32_t>("content/models/wolf/wolfman_animated.fbx", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS),
		    std::make_pair<std::string, uint32_t>("content/models/multiple_skeleton/twocubestest.gltf", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS | rawrbox::ModelLoadFlags::Debug::PRINT_BONE_STRUCTURE),
		    std::make_pair<std::string, uint32_t>("content/models/grandma_tv/scene.gltf", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS | rawrbox::ModelLoadFlags::Debug::PRINT_MATERIALS)};

		rawrbox::ASYNC::run([initialContentFiles]() {
			for (auto& f : initialContentFiles) {
				rawrbox::RESOURCES::loadFile(f.first, f.second);
			} }, [this] { rawrbox::runOnMainThread([this]() {
										  rawrbox::RESOURCES::upload();
										  this->contentLoaded();
									  }); });

		this->_window->upload();

		// DEBUG ---
		rawrbox::GIZMOS::upload();
		// -----------
	}

	void Game::contentLoaded() {
		this->_font = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>("cour.ttf")->getSize(16);

		// Assimp test ---
		auto mdl = rawrbox::RESOURCES::getFile<rawrbox::ResourceAssimp>("./content/models/ps1_phasmophobia/Phasmaphobia_Semi.fbx");

		this->_model->load(mdl->model);
		this->_model->setPos({7, 1.1F, 0.F});

		this->_model2->load(mdl->model);
		this->_model2->setPos({-6, 1.1F, 0.F});

		// ANIMATIONS ---
		auto mdl2 = rawrbox::RESOURCES::getFile<rawrbox::ResourceAssimp>("./content/models/wolf/wolfman_animated.fbx");
		this->_model3->load(mdl2->model);
		this->_model3->playAnimation("Scene", true, 1.F);
		this->_model3->setPos({0, 0, 0});

		auto mdl3 = rawrbox::RESOURCES::getFile<rawrbox::ResourceAssimp>("./content/models/multiple_skeleton/twocubestest.gltf");
		this->_model4->load(mdl3->model);
		this->_model4->playAnimation("MewAction", true, 0.8F);
		this->_model4->playAnimation("MewAction.001", true, 0.5F);
		this->_model4->setPos({0, 0, 2.5F});
		this->_model4->setScale({0.25F, 0.25F, 0.25F});

		auto mdl4 = rawrbox::RESOURCES::getFile<rawrbox::ResourceAssimp>("./content/models/grandma_tv/scene.gltf");
		this->_model5->load(mdl4->model);
		this->_model5->playAnimation("Scene", true, 1.F);
		this->_model5->setPos({0, 0, -3.5F});
		this->_model5->setScale({0.35F, 0.35F, 0.35F});
		this->_model5->setEulerAngle({0, bx::toRad(90.F), 0});

		//   -----

		// Text test ----
		{
			auto f = this->_font.lock();
			this->_text->addText(f, "TEXTURES + LIGHT", {-6.F, 3.0F, 0});
			this->_text->addText(f, "TEXTURES", {6.F, 3.0F, 0});
			this->_text->addText(f, "SINGLE ARMATURE +\nVERTEX ANIMATION", {0.F, 2.F, 0});
			this->_text->addText(f, "TWO ARMATURES +\nTWO ANIMATIONS", {0.F, 1.F, 2.5F});
			this->_text->addText(f, "VERTEX ANIMATIONS", {0.F, 1.8F, -3.5F});
			this->_text->upload();
		}
		// ------

		{
			auto mesh = this->_modelGrid->generateGrid(24, {0.F, 0.F, 0.F});
			this->_modelGrid->addMesh(mesh);
			this->_modelGrid->upload();
		}

		this->_ready = true;
	}

	void Game::shutdown() {
		this->_window.reset();
		this->_camera.reset();

		this->_model.reset();
		this->_model2.reset();
		this->_model3.reset();
		this->_model4.reset();
		this->_model5.reset();
		this->_modelGrid.reset();

		this->_text.reset();

		rawrbox::GIZMOS::shutdown();
		rawrbox::RESOURCES::shutdown();
		rawrbox::LIGHTS::shutdown();
		rawrbox::Engine::shutdown();
	}

	void Game::pollEvents() {
		if (this->_window == nullptr) return;
		this->_window->pollEvents();
	}

	void Game::update() {
		if (this->_camera == nullptr) return;
		this->_camera->update();
	}

	void Game::drawWorld() {
		auto pos = this->_camera->getPos();
		this->_modelGrid->draw(pos);

		this->_model->draw(pos);
		this->_model2->draw(pos);
		this->_model3->draw(pos);
		this->_model4->draw(pos);
		this->_model5->draw(pos);

		this->_text->draw(pos);
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
		bgfx::dbgTextPrintf(1, 1, 0x1f, "003-assimp-loading");
		bgfx::dbgTextPrintf(1, 2, 0x3f, "Description: ASSIMP model loading test");
		printFrames();
		// -----------

		if (this->_ready) {
			this->drawWorld();
		} else {
			bgfx::dbgTextPrintf(1, 10, 0x70, "                                   ");
			bgfx::dbgTextPrintf(1, 11, 0x70, "          LOADING CONTENT          ");
			bgfx::dbgTextPrintf(1, 12, 0x70, "                                   ");
		}

		// Draw DEBUG ---
		rawrbox::GIZMOS::draw();
		// -----------

		this->_window->frame(); // Commit primitives
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, this->_camera->getViewMtx().data(), this->_camera->getProjMtx().data());
	}
} // namespace assimp
