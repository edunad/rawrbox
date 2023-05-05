
#include <rawrbox/bass/manager.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/utils/keys.hpp>

#include <bass_test/game.hpp>

#include <bx/bx.h>
#include <bx/math.h>
#include <fmt/printf.h>

#include <vector>

#include "rawrbox/bass/sound/flags.hpp"

namespace bass_test {
	void Game::init() {
		int width = 1024;
		int height = 768;

		this->_window = std::make_unique<rawrBox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("BASS TEST");
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
		this->_render->setClearColor(0x00000000);

		// Setup camera
		this->_camera = std::make_shared<rawrBox::CameraPerspective>(this->_window->getAspectRatio(), 60.0F, 0.1F, 100.0F, bgfx::getCaps()->homogeneousDepth);
		this->_camera->setPos({0.F, 5.F, -5.F});
		this->_camera->setAngle({0.F, 0.F, bx::toRad(-45), 0.F});
		// --------------

		rawrBox::SoundManager::get().initialize();
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

		// SOUND -----
		// https://i.rawr.dev/Mystery%20Skulls%20-%20Freaking%20Out.mp3
		// https://i.rawr.dev/Just_a_Bit_Crazy.ogg
		this->_sound = rawrBox::SoundManager::get().loadHTTPSound("https://i.rawr.dev/Just_a_Bit_Crazy.ogg", rawrBox::SoundFlags::SOUND_3D | rawrBox::SoundFlags::BEAT_DETECTION | rawrBox::SoundFlags::BPM_DETECTION)->createInstance();
		this->_sound->setVolume(1.F);
		this->_sound->setLooping(true);
		this->_sound->set3D(10.F);
		this->_sound->setPosition({0, 1.F, 0});
		this->_sound->setTempo(0.8F);
		this->_sound->play();

		// this->_sound->setBeatSettings(8, 8, 2.f);
		this->_sound->onBEAT += [this](double pos) {
			this->_beat = 0.5F;
		};

		this->_sound->onBPM += [](float bpm) {
			fmt::print("BPM: {}\n", bpm);
		};
		// --------

		// Text test ----
		{
			this->_text->addText(this->_font, "BEAT", {0.F, 1.3F, 0});
			this->_text->upload();
		}
		// ------

		// GRID -----
		{
			auto mesh = this->_modelGrid->generateGrid(12, {0.F, 0.F, 0.F});
			this->_modelGrid->addMesh(mesh);
			this->_modelGrid->upload();
		}
	}

	void Game::shutdown() {
		this->_render = nullptr;
		this->_sound = nullptr;
		this->_modelGrid = nullptr;

		rawrBox::SoundManager::get().shutdown();
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

		rawrBox::SoundManager::get().setListenerLocation(this->_camera->getPos(), this->_camera->getForward(), this->_camera->getUp());
		if (this->_beat > 0.F) this->_beat -= 0.05F;
	}

	void Game::drawWorld() {
		bgfx::setViewTransform(rawrBox::CURRENT_VIEW_ID, this->_camera->getViewMtx().data(), this->_camera->getProjMtx().data());

		this->_text->setPos({0, this->_beat, 0});
		this->_text->draw({});
		this->_modelGrid->draw({});
	}

	void Game::draw(const double alpha) {
		if (this->_render == nullptr) return;
		this->_render->swapBuffer(); // Clean up and set renderer

		bgfx::setViewTransform(rawrBox::CURRENT_VIEW_ID, this->_camera->getViewMtx().data(), this->_camera->getProjMtx().data());
		bgfx::dbgTextPrintf(1, 1, 0x0f, "BASS TESTS -----------------------------------------------------------------------------------------------------------");

		this->drawWorld();

		this->_render->render(true); // Commit primitives
	}
} // namespace bass_test
