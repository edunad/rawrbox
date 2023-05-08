
#include <rawrbox/render/particles/emitter.hpp>
#include <rawrbox/render/static.hpp>

#include <particle_test/game.hpp>

#include <bx/bx.h>
#include <bx/math.h>
#include <fmt/printf.h>

#include <memory>
#include <vector>

namespace particle_test {
	void Game::init() {
		int width = 1024;
		int height = 768;

		this->_window = std::make_unique<rawrbox::Window>();
		this->_window->setMonitor(1);
		this->_window->setTitle("PARTICLE TEST");
		this->_window->setRenderer(bgfx::RendererType::Count);
		this->_window->onResize += [this](auto& w, auto& size) {
			if (this->_render == nullptr) return;
			this->_render->resizeView(size);
		};

		this->_window->onWindowClose += [this](auto& w) {
			this->shutdown();
		};

		this->_window->initialize(width, height, rawrbox::WindowFlags::Debug::TEXT | rawrbox::WindowFlags::Window::WINDOWED);

		this->_render = std::make_shared<rawrbox::Renderer>(0, this->_window->getSize());
		this->_render->setClearColor(0x00000000);

		// Setup camera
		this->_camera = std::make_shared<rawrbox::CameraOrbital>(this->_window.get());
		this->_camera->setPos({0.F, 5.F, -5.F});
		this->_camera->setAngle({0.F, 0.F, bx::toRad(-45), 0.F});
		// --------------

		this->_textEngine = std::make_unique<rawrbox::TextEngine>();

		// Load content ---
		this->loadContent();
		// -----
	}

	int64_t m_timeOffset;
	void Game::loadContent() {
		this->_render->upload();

		// TEXTURES ----
		this->_texture = std::make_shared<rawrbox::TextureImage>("./content/textures/particles/circle_05.png");
		this->_texture->upload();
		// -----

		// Fonts -----
		this->_font = &this->_textEngine->load("cour.ttf", 16);
		// ------

		// Setup Engine
		this->_ps = std::make_shared<rawrbox::ParticleSystem<>>();
		this->_ps->setTexture(this->_texture);

		{
			auto m = std::make_shared<rawrbox::Emitter>();
			m->setPos({-4, 0.5F, 0});

			this->_ps->addEmitter(m);
		}

		{
			rawrbox::EmitterSettings s;
			s.offsetStart = {0.F, 0.05F};
			s.rgba = {0xFF0000FF, 0xFF000FFF, 0xFF00F0FF, 0xFF000000, 0x00FFFFFF};
			s.scaleEnd = {0.5F, 0.5F};
			s.offsetEnd = {0.45F, 0.45F};
			s.preHeat = true;
			s.particlesPerSecond = 20;
			s.maxParticles = 100;

			auto m = std::make_shared<rawrbox::Emitter>(s);
			m->setPos({2, 0.5F, 0});

			this->_ps->addEmitter(m);
		}

		this->_ps->upload();

		// ------

		{
			this->_text->addText(this->_font, "DEFAULT EMITTER SETTINGS", {-4.F, 0.15F, 0});
			this->_text->upload();
		}

		// GRID -----
		{
			auto mesh = this->_modelGrid->generateGrid(12, {0.F, 0.F, 0.F});
			this->_modelGrid->addMesh(mesh);
			this->_modelGrid->upload();
		}
	}

	void Game::shutdown() {
		this->_render = nullptr;
		this->_camera = nullptr;
		this->_ps = nullptr;
		this->_modelGrid = nullptr;
		this->_text = nullptr;
		this->_font = nullptr;
		this->_textEngine = nullptr;

		rawrbox::Engine::shutdown();
	}

	void Game::pollEvents() {
		if (this->_window == nullptr) return;
		this->_window->pollEvents();
	}

	void Game::update(float deltaTime, int64_t gameTime) {
		if (this->_camera == nullptr || this->_ps == nullptr) return;

		this->_camera->update(deltaTime);
		this->_ps->update(deltaTime);
	}

	void Game::drawWorld() {
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, this->_camera->getViewMtx().data(), this->_camera->getProjMtx().data());

		this->_modelGrid->draw({});
		this->_ps->draw(this->_camera);
		this->_text->draw({});
	}

	void Game::draw() {
		if (this->_render == nullptr) return;
		this->_render->swapBuffer(); // Clean up and set renderer

		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, this->_camera->getViewMtx().data(), this->_camera->getProjMtx().data());
		bgfx::dbgTextPrintf(1, 1, 0x0f, "PARTICLE TESTS -----------------------------------------------------------------------------------------------------------");

		this->drawWorld();

		this->_render->render(true); // Commit primitives
	}
} // namespace particle_test
