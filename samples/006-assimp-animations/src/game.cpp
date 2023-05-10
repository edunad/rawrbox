
#include <rawrbox/render/model/mesh.hpp>
#include <rawrbox/utils/keys.hpp>

#include <anims/game.hpp>

#include <bx/bx.h>
#include <bx/math.h>
#include <bx/timer.h>

#include <vector>

namespace anims {
	void Game::init() {
		int width = 1024;
		int height = 768;

		this->_window = std::make_unique<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("ANIMATIONS TEST");
		this->_window->setRenderer(bgfx::RendererType::Count);
		this->_window->onResize += [this](auto& w, auto& size) {
			if (this->_render == nullptr) return;
			this->_render->resizeView(size);
		};

		this->_window->onWindowClose += [this](auto& w) {
			this->shutdown();
		};

		this->_window->initialize(width, height, rawrbox::WindowFlags::Window::WINDOWED | rawrbox::WindowFlags::Debug::TEXT);

		this->_render = std::make_shared<rawrbox::Renderer>(0, this->_window->getSize());
		this->_render->setClearColor(0x00000000);

		// Setup camera
		this->_camera = std::make_shared<rawrbox::CameraOrbital>(this->_window.get());
		this->_camera->setPos({0.F, 5.F, -5.F});
		this->_camera->setAngle({0.F, bx::toRad(-45), 0.F, 0.F});
		// --------------

		this->_textEngine = std::make_unique<rawrbox::TextEngine>();

		// Load content ---
		this->loadContent();
		// -----
	}

	void Game::loadContent() {
		this->_render->upload();

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
		this->_model2->setPos({0, 0, 1.5F});
		this->_model2->setScale({0.25F, 0.25F, 0.25F});
		this->_model2->upload();
		// -----

		{
			auto mesh = this->_modelGrid->generateGrid(12, {0.F, 0.F, 0.F});
			this->_modelGrid->addMesh(mesh);
		}

		// Text test ----
		{
			this->_text->addText(this->_font, "SINGLE ARMATURE", {0.F, 1.8F, 0});
			this->_text->addText(this->_font, "TWO ARMATURES", {0.F, 1.8F, 2.3F});
		}
		// ------

		this->_text->upload();
		this->_modelGrid->upload();
		// -----
	}

	void Game::shutdown() {
		this->_render = nullptr;
		this->_camera = nullptr;

		this->_text = nullptr;
		this->_model = nullptr;
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
		if (this->_model == nullptr || this->_modelGrid == nullptr || this->_model2 == nullptr || this->_text == nullptr) return;

		this->_modelGrid->draw(this->_camera->getPos());

		this->_model->draw(this->_camera->getPos());
		this->_model2->draw(this->_camera->getPos());
		this->_text->draw(this->_camera->getPos());
	}

	void printFrames() {
		int64_t now = bx::getHPCounter();
		static int64_t last = now;
		const int64_t frameTime = now - last;
		last = now;

		const auto freq = static_cast<double>(bx::getHPFrequency());
		const double toMs = 1000.0 / freq;

		bgfx::dbgTextPrintf(1, 4, 0x0f, "Frame: %7.3f[ms]", double(frameTime) * toMs);
	}

	void Game::draw() {
		if (this->_render == nullptr) return;
		this->_render->clear(); // Clean up and set renderer

		// DEBUG ----
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(1, 1, 0x1f, "006-assimp-animations");
		bgfx::dbgTextPrintf(1, 2, 0x3f, "Description: ASSIMP animation test");
		printFrames();
		// -----------

		this->drawWorld();

		this->_render->frame(); // Commit primitives
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, this->_camera->getViewMtx().data(), this->_camera->getProjMtx().data());
	}
} // namespace anims
