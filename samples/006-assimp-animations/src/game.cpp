
#include <rawrbox/render/model/light/manager.hpp>
#include <rawrbox/render/model/mesh.hpp>
#include <rawrbox/utils/keys.hpp>

#include <anims/game.hpp>

#include <bx/bx.h>
#include <bx/math.h>

#include <vector>

namespace anims {
	void Game::init() {
		int width = 1024;
		int height = 768;

		this->_window = std::make_shared<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("ANIMATIONS TEST");
		this->_window->setRenderer(bgfx::RendererType::Count);
		this->_window->onWindowClose += [this](auto& w) {
			this->shutdown();
		};

		this->_window->initialize(width, height, rawrbox::WindowFlags::Window::WINDOWED | rawrbox::WindowFlags::Debug::TEXT | rawrbox::WindowFlags::Debug::PROFILER);

		// Setup camera
		this->_camera = std::make_shared<rawrbox::CameraOrbital>(this->_window);
		this->_camera->setPos({0.F, 5.F, -5.F});
		this->_camera->setAngle({0.F, bx::toRad(-45), 0.F, 0.F});
		// --------------

		this->_textEngine = std::make_unique<rawrbox::TextEngine>();

		// Load content ---
		this->loadContent();
		// -----
	}

	void Game::loadContent() {
		this->_window->upload();

		// Fonts -----
		this->_font = &this->_textEngine->load("cour.ttf", 16);
		// ------

		// Assimp test ---
		this->_model->load("./content/models/wolf/wolfman_animated.fbx", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS);
		this->_model->playAnimation("Scene", true, 1.F);
		this->_model->setPos({0, 0, 0});
		this->_model->upload();

		this->_model2->load("./content/models/multiple_skeleton/twocubestest.gltf", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS | rawrbox::ModelLoadFlags::Debug::PRINT_BONE_STRUCTURE);
		this->_model2->playAnimation("MewAction", true, 0.8F);
		this->_model2->playAnimation("MewAction.001", true, 0.5F);
		this->_model2->setPos({0, 0, 2.5F});
		this->_model2->setScale({0.25F, 0.25F, 0.25F});
		this->_model2->upload();

		this->_model3->load("./content/models/grandma_tv/TV_emission.fbx", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS);
		this->_model3->playAnimation("Scene", true, 1.F);
		this->_model3->setPos({0, 0, -3.5F});
		this->_model3->setScale({0.35F, 0.35F, 0.35F});
		this->_model3->upload();
		// -----

		{
			auto mesh = this->_modelGrid->generateGrid(12, {0.F, 0.F, 0.F});
			this->_modelGrid->addMesh(mesh);
		}

		// Text test ----
		{
			this->_text->addText(this->_font, "SINGLE ARMATURE +\nVERTEX ANIMATION", {0.F, 1.8F, 0});
			this->_text->addText(this->_font, "TWO ARMATURES +\nTWO ANIMATIONS", {0.F, 1.8F, 3.3F});
			this->_text->addText(this->_font, "VERTEX ANIMATIONS", {0.F, 1.8F, -3.5F});
		}
		// ------

		this->_text->upload();
		this->_modelGrid->upload();
		// -----
	}

	void Game::shutdown() {
		this->_window = nullptr;
		this->_camera = nullptr;

		this->_text = nullptr;

		this->_model = nullptr;
		this->_model2 = nullptr;
		this->_model3 = nullptr;
		this->_modelGrid = nullptr;

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
		if (this->_model == nullptr || this->_model2 == nullptr || this->_model3 == nullptr || this->_modelGrid == nullptr || this->_text == nullptr) return;

		this->_modelGrid->draw(this->_camera->getPos());

		this->_model->draw(this->_camera->getPos());
		this->_model2->draw(this->_camera->getPos());
		this->_model3->draw(this->_camera->getPos());

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
		bgfx::dbgTextPrintf(1, 1, 0x1f, "006-assimp-animations");
		bgfx::dbgTextPrintf(1, 2, 0x3f, "Description: ASSIMP animation test");
		printFrames();
		// -----------

		this->drawWorld();

		this->_window->frame(true); // Commit primitives
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, this->_camera->getViewMtx().data(), this->_camera->getProjMtx().data());
	}
} // namespace anims
