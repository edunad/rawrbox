#include <rawrbox/render/model/light/manager.hpp>
#include <rawrbox/utils/keys.hpp>

#include <assimp/game.hpp>

#include <bx/bx.h>
#include <bx/math.h>

#include <vector>

namespace assimp {
	void Game::init() {
		int width = 1024;
		int height = 768;

		this->_window = std::make_unique<rawrBox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("ASSIMP TEST");
		this->_window->setRenderer(bgfx::RendererType::Count);
		this->_window->onResize += [this](auto& w, auto& size) {
			if (this->_render == nullptr) return;
			this->_render->resizeView(size);
		};

		this->_window->onWindowClose += [this](auto& w) {
			this->shutdown();
		};

		this->_window->onMouseKey += [this](auto& w, const rawrBox::Vector2i& mousePos, int button, int action, int mods) {
			const bool isDown = action == 1;
			if (button != MOUSE_BUTTON_2) return;

			this->_rightClick = isDown;
			this->_oldMousePos = mousePos;
		};

		this->_window->onMouseMove += [this](auto& w, const rawrBox::Vector2i& mousePos) {
			if (this->_camera == nullptr || !this->_rightClick) return;

			float m_mouseSpeed = 0.0015F;

			auto deltaX = mousePos.x - this->_oldMousePos.x;
			auto deltaY = mousePos.y - this->_oldMousePos.y;

			auto ang = this->_camera->getAngle();
			ang.x += m_mouseSpeed * static_cast<float>(deltaX);
			ang.y -= m_mouseSpeed * static_cast<float>(deltaY);

			this->_camera->setAngle(ang);
			this->_oldMousePos = mousePos;
		};

		this->_window->initialize(width, height, rawrBox::WindowFlags::Debug::TEXT | rawrBox::WindowFlags::Window::WINDOWED);

		this->_render = std::make_shared<rawrBox::Renderer>(0, this->_window->getSize());
		this->_render->setClearColor(0x000000FF);

		// Setup camera
		this->_camera = std::make_shared<rawrBox::CameraPerspective>(this->_window->getAspectRatio(), 60.0F, 0.1F, 100.0F, bgfx::getCaps()->homogeneousDepth);
		this->_camera->setPos({0.F, 5.F, -5.F});
		this->_camera->setAngle({0.F, 0.F, bx::toRad(-45), 0.F});
		// --------------

		this->_textEngine = std::make_unique<rawrBox::TextEngine>();

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
		this->_model->setPos({10, 0, 0});
		this->_model->load("./content/models/ps1_phasmophobia/Phasmaphobia_Semi.fbx");
		this->_model->upload();

		this->_model2->setPos({0, 0, 0});
		this->_model2->load("./content/models/ps1_phasmophobia/Phasmaphobia_Semi.fbx", rawrBox::ModelLoadFlags::IMPORT_TEXTURES);
		this->_model2->upload();

		this->_model3->setPos({-10, 0, 0});
		this->_model3->load("./content/models/ps1_phasmophobia/Phasmaphobia_Semi.fbx", rawrBox::ModelLoadFlags::IMPORT_TEXTURES | rawrBox::ModelLoadFlags::IMPORT_LIGHT);
		this->_model3->upload();
		// -----

		// Text test ----
		{
			this->_text->addText(this->_font, "TEXTURES + LIGHT", {-10.F, 2.0F, 0});
			this->_text->addText(this->_font, "NONE", {10.F, 2.0F, 0});
			this->_text->addText(this->_font, "TEXTURES", {0.F, 2.0F, 0});
			this->_text->upload();
		}
		// ------
	}

	void Game::shutdown() {
		this->_render = nullptr;

		this->_model = nullptr;
		this->_model2 = nullptr;
		this->_model3 = nullptr;

		this->_text = nullptr;

		rawrBox::LightManager::get().destroy();
		rawrBox::Engine::shutdown();
	}

	void Game::pollEvents() {
		if (this->_window == nullptr) return;
		this->_window->pollEvents();
	}

	void Game::update(float deltaTime, int64_t gameTime) {
		if (this->_render == nullptr || this->_camera == nullptr) return;

		float m_moveSpeed = 10.F;

		auto dir = this->_camera->getForward();
		auto eye = this->_camera->getPos();
		auto right = this->_camera->getRight();

		auto m_dir = bx::Vec3(dir.x, dir.y, dir.z);
		auto m_eye = bx::Vec3(eye.x, eye.y, eye.z);

		if (this->_window->isKeyDown(KEY_LEFT_SHIFT)) {
			m_moveSpeed = 60.F;
		}

		if (this->_window->isKeyDown(KEY_W)) {
			m_eye = bx::mad(m_dir, deltaTime * m_moveSpeed, m_eye);
			this->_camera->setPos({m_eye.x, m_eye.y, m_eye.z});
		}

		if (this->_window->isKeyDown(KEY_S)) {
			m_eye = bx::mad(m_dir, -deltaTime * m_moveSpeed, m_eye);
			this->_camera->setPos({m_eye.x, m_eye.y, m_eye.z});
		}

		if (this->_window->isKeyDown(KEY_A)) {
			m_eye = bx::mad({right.x, right.y, right.z}, deltaTime * m_moveSpeed, m_eye);
			this->_camera->setPos({m_eye.x, m_eye.y, m_eye.z});
		}

		if (this->_window->isKeyDown(KEY_D)) {
			m_eye = bx::mad({right.x, right.y, right.z}, -deltaTime * m_moveSpeed, m_eye);
			this->_camera->setPos({m_eye.x, m_eye.y, m_eye.z});
		}
	}

	void Game::drawWorld() {
		if (this->_model == nullptr || this->_model2 == nullptr || this->_model3 == nullptr || this->_text == nullptr) return;
		auto pos = this->_camera->getPos();

		this->_model->draw(pos);
		this->_model2->draw(pos);
		this->_model3->draw(pos);

		this->_text->draw(pos);
	}

	void Game::draw(const double alpha) {
		if (this->_render == nullptr) return;
		this->_render->swapBuffer(); // Clean up and set renderer

		bgfx::setViewTransform(rawrBox::CURRENT_VIEW_ID, this->_camera->getViewMtx().data(), this->_camera->getProjMtx().data());
		bgfx::dbgTextPrintf(1, 1, 0x0f, "ASSIMP TESTS ----------------------------------------------------------------------------------------------------------------");

		this->drawWorld();

		this->_render->render(true); // Commit primitives
	}
} // namespace assimp
