
#include <rawrbox/debug/gizmos.hpp>
#include <rawrbox/render/particles/emitter.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/resources/manager.hpp>

#include <particle_test/game.hpp>

#include <bx/bx.h>
#include <bx/math.h>
#include <fmt/printf.h>

#include <memory>
#include <vector>

namespace particle_test {
	void Game::setupGLFW() {
		this->_window = std::make_shared<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("PARTICLE TEST");
		this->_window->setRenderer(bgfx::RendererType::Count);
		this->_window->create(1024, 768, rawrbox::WindowFlags::Debug::TEXT | rawrbox::WindowFlags::Debug::PROFILER | rawrbox::WindowFlags::Window::WINDOWED | rawrbox::WindowFlags::Features::MULTI_THREADED);
		this->_window->onWindowClose += [this](auto& w) { this->shutdown(); };
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
		rawrbox::RESOURCES::addLoader(std::make_unique<rawrbox::TextureLoader>());

		// Load content ---
		this->loadContent();
		// -----
	}

	void Game::loadContent() {
		std::array initialContentFiles = {
		    std::make_pair<std::string, uint32_t>("cour.ttf", 0),
		    std::make_pair<std::string, uint32_t>("content/textures/particles/particles.png", 0)};

		rawrbox::ASYNC::run([initialContentFiles]() {
			for (auto& f : initialContentFiles) {
				rawrbox::RESOURCES::loadFile(f.first, f.second);
			} }, [this] { rawrbox::runOnRenderThread([this]() {
										  rawrbox::RESOURCES::upload();
										  this->contentLoaded();
									  }); });

		this->_window->upload();

		// DEBUG ---
		rawrbox::GIZMOS::upload();
		// -----------
	}

	void Game::contentLoaded() {
		// Fonts -----
		this->_font = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>("cour.ttf")->getSize(16);
		// ------

		// Setup Engine
		auto texture = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("content/textures/particles/particles.png")->texture;
		this->_ps = std::make_shared<rawrbox::ParticleSystem<>>(texture);

		{
			auto m = std::make_shared<rawrbox::Emitter>();
			m->setPos({-2.F, 0.5F, 0});

			this->_ps->addEmitter(m);
		}

		{
			rawrbox::EmitterSettings s;
			s.texture = {0, 3};
			s.offsetStart = {0.F, 0.05F};
			s.rgba = {0xFF0000FF, 0xFF000FFF, 0xFF00F0FF, 0xFF000000, 0x00FFFFFF};
			s.scaleEnd = {0.5F, 0.5F};
			s.offsetEnd = {0.85F, 0.85F};
			s.preHeat = true;
			s.particlesPerSecond = 20;
			s.maxParticles = 100;

			this->_em = std::make_shared<rawrbox::Emitter>(s);
			this->_ps->addEmitter(this->_em);
		}

		this->_ps->upload();

		// ------

		{
			auto f = this->_font.lock();
			this->_text->addText(f, "DEFAULT EMITTER SETTINGS", {-2.F, 0.15F, 0});
			this->_text->addText(f, "CUSTOM EMITTER SETTINGS", {2.F, 0.15F, 0});
			this->_text->upload();
		}

		// GRID -----
		{
			auto mesh = this->_modelGrid->generateGrid(12, {0.F, 0.F, 0.F});
			this->_modelGrid->addMesh(mesh);
			this->_modelGrid->upload();
		}

		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) return;

		this->_window.reset();
		this->_camera.reset();
		this->_ps.reset();
		this->_modelGrid.reset();
		this->_text.reset();

		rawrbox::GIZMOS::shutdown();
		rawrbox::RESOURCES::shutdown();
		rawrbox::ASYNC::shutdown();
	}

	void Game::pollEvents() {
		if (this->_window == nullptr) return;
		this->_window->pollEvents();
	}

	float move = 0.F;
	void Game::update() {
		if (this->_camera == nullptr || this->_ps == nullptr) return;

		this->_camera->update();
		this->_ps->update();

		this->_em->setPos({2.F + std::cos(move) * 0.5F, 0.5F, std::sin(move) * 0.5F});
		move += 0.05F;
	}

	void Game::drawWorld() {
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, this->_camera->getViewMtx().data(), this->_camera->getProjMtx().data());

		this->_modelGrid->draw({});
		this->_ps->draw(this->_camera);
		this->_text->draw({});
	}

	void Game::printFrames() {
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
		bgfx::dbgTextPrintf(1, 1, 0x1f, "007-particle-system");
		bgfx::dbgTextPrintf(1, 2, 0x3f, "Description: Particle system test");
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
} // namespace particle_test
