
#include <rawrbox/engine/static.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/render/resources/gif.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/resources/manager.hpp>

#include <stencil/game.hpp>

#include <bx/bx.h>
#include <bx/math.h>
#include <fmt/format.h>

#include <vector>

namespace stencil {
	void Game::setupGLFW() {
		this->_window = std::make_shared<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("STENCIL TEST");
		this->_window->setClearColor(0x443355FF);
		this->_window->setRenderer(bgfx::RendererType::Count);
		this->_window->create(1024, 768, rawrbox::WindowFlags::Debug::TEXT | rawrbox::WindowFlags::Debug::PROFILER | rawrbox::WindowFlags::Window::WINDOWED | rawrbox::WindowFlags::Features::MULTI_THREADED);
		this->_window->onWindowClose += [this](auto& w) { this->shutdown(); };
	}

	void Game::init() {
		if (this->_window == nullptr) return;
		this->_window->initializeBGFX();

		rawrbox::RESOURCES::addLoader(std::make_unique<rawrbox::TextureLoader>());
		rawrbox::RESOURCES::addLoader(std::make_unique<rawrbox::GIFLoader>());
		rawrbox::RESOURCES::addLoader(std::make_unique<rawrbox::FontLoader>());

		// Load content ---
		this->loadContent();
		// -----
	}

	void Game::loadContent() {
		std::array<std::string, 5> initialContentFiles = {
		    "content/fonts/droidsans.ttf",
		    "content/fonts/visitor1.ttf",
		    "cour.ttf",
		    "content/textures/screem.png",
		    "content/textures/meow3.gif",
		};

		for (auto& f : initialContentFiles) {
			this->_loadingFiles++;

			rawrbox::RESOURCES::loadFileAsync(f, 0, [this]() {
				this->_loadingFiles--;
				if (this->_loadingFiles <= 0) {
					rawrbox::runOnRenderThread([this]() { this->contentLoaded(); });
				}
			});
		}

		this->_window->upload();
	}

	void Game::contentLoaded() {
		// Textures ---
		this->_texture = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./content/textures/screem.png")->texture;
		this->_texture2 = rawrbox::RESOURCES::getFile<rawrbox::ResourceGIF>("./content/textures/meow3.gif")->texture;

		this->_font = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>("./content/fonts/droidsans.ttf")->getSize(28);
		this->_font2 = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>("./content/fonts/visitor1.ttf")->getSize(18);
		this->_font3 = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>("cour.ttf")->getSize(12);

		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) return;

		this->_texture.reset();
		this->_texture2.reset();

		this->_font = nullptr;
		this->_font2 = nullptr;
		this->_font3 = nullptr;

		rawrbox::RESOURCES::shutdown();
		rawrbox::ASYNC::shutdown();

		this->_window->unblockPoll();
		this->_window.reset();
	}

	void Game::pollEvents() {
		if (this->_window == nullptr) return;
		this->_window->pollEvents();
	}

	void Game::drawOverlay() {
		auto& stencil = this->_window->getStencil();

		stencil.pushOffset({20, 50});

		// Box + clipping --
		stencil.pushRotation({this->_counter * 50.5F, {50, 50}});
		stencil.pushClipping({-20, -20, 50, 140});
		stencil.drawBox({0, 0}, {100, 100}, rawrbox::Colors::Green);
		stencil.popClipping();

		stencil.pushClipping({50, -20, 50, 140});
		stencil.drawBox({0, 0}, {100, 100}, rawrbox::Colors::Red);
		stencil.popClipping();
		stencil.popRotation();
		// --

		// Inverted box ---
		stencil.pushOffset({100, 0});
		stencil.pushScale({1.F, -1.F});
		stencil.drawBox({0, 0}, {100, 100}, rawrbox::Colors::Red);
		stencil.popScale();
		stencil.popOffset();
		// ---

		// Outline box ---
		stencil.pushOffset({200, 0});
		stencil.pushOutline({1.F, 2.F});
		stencil.drawBox({0, 0}, {100, 100}, rawrbox::Colors::Purple);
		stencil.popOutline();

		stencil.pushOutline({2.F});
		stencil.drawBox({25, 25}, {50, 50}, rawrbox::Colors::Purple);
		stencil.popOutline();
		stencil.popOffset();
		// ---

		// Triangle ---
		stencil.pushOffset({300, 0});
		stencil.drawTriangle({0, 0}, {0, 0}, rawrbox::Colors::Blue, {0, 100}, {0, 1}, rawrbox::Colors::Blue, {100, 0}, {0, 1}, rawrbox::Colors::Blue);
		stencil.popOffset();
		// ---

		// Outline triangle ---
		stencil.pushOffset({400, 0});
		stencil.pushOutline({2.F});
		stencil.drawTriangle({15, 15}, {0, 0}, rawrbox::Colors::Blue, {15, 65}, {0, 1}, rawrbox::Colors::Blue, {65, 15}, {0, 1}, rawrbox::Colors::Blue);
		stencil.popOutline();

		stencil.pushOutline({1.F, 1.F});
		stencil.drawTriangle({0, 0}, {0, 0}, rawrbox::Colors::Blue, {0, 100}, {0, 1}, rawrbox::Colors::Blue, {100, 0}, {0, 1}, rawrbox::Colors::Blue);
		stencil.popOutline();
		stencil.popOffset();
		// ---

		// Circle ---
		stencil.pushOffset({500, 0});
		stencil.drawCircle({0, 0}, {100, 100}, rawrbox::Colors::Orange, 16, 0, std::fmod(this->_counter * 50.5F, 360.F));
		stencil.popOffset();
		// ---

		// Outline circle ---
		stencil.pushOffset({600, 0});
		stencil.pushOutline({1.F, 0.25F});
		stencil.drawCircle({0, 0}, {100, 100}, rawrbox::Colors::Red, 16, 0.F, std::fmod(this->_counter * 50.5F, 360.F));
		stencil.popOutline();

		stencil.pushOutline({2.F});
		stencil.drawCircle({25, 25}, {50, 50}, rawrbox::Colors::Red, 32, 0.F, std::fmod(this->_counter * 50.5F, 360.F));
		stencil.popOutline();
		stencil.popOffset();
		// ---

		// Line ---
		stencil.pushOffset({700, 0});
		stencil.drawLine({0, 0}, {100, 100}, rawrbox::Colors::Red);
		// ---

		// Outline line ---
		stencil.pushOutline({1.F, 2.F});
		stencil.drawLine({100, 0}, {0, 100}, rawrbox::Colors::Blue);
		stencil.popOutline();

		stencil.pushOutline({3.F, 2.F});
		stencil.drawLine({50, 0}, {50, 100}, rawrbox::Colors::Purple);
		stencil.popOutline();
		stencil.popOffset();
		// ---

		// Texture ---
		stencil.pushOffset({800, 0});
		stencil.drawTexture({0, 0}, {100, 100}, this->_texture);
		stencil.popOffset();

		stencil.pushOffset({900, 0});
		stencil.drawTexture({0, 0}, {100, 100}, this->_texture2);
		stencil.popOffset();
		// ---

		// POLYGON ---
		rawrbox::Polygon poly;
		poly.verts = {
		    {{0, 0}, {0, 0}, rawrbox::Colors::White},
		    {{0, 100}, {1, 0}, rawrbox::Colors::White},
		    {{100, 0}, {0, 1}, rawrbox::Colors::White},
		    {{80, 80}, {1, 1}, rawrbox::Colors::White}};
		poly.indices = {0, 1, 2,
		    1, 3, 2};

		stencil.pushOffset({0, 260});
		stencil.drawPolygon(poly);

		stencil.pushOutline({1.F, 2.F});
		stencil.pushOffset({140, 0});
		stencil.drawPolygon(poly);
		stencil.popOffset();
		stencil.popOutline();

		stencil.popOffset();

		// -----

		stencil.popOffset();

		// Text ---
		stencil.pushOffset({20, 200});
		stencil.drawText(*this->_font, "Cat ipsum dolor sit amet, steal raw zucchini off kitchen counter. $£%&", {});

		auto size = this->_font2->getStringSize("Cat!!");

		stencil.pushRotation({this->_counter * 50.5F, (size / 2.F) + rawrbox::Vector2f(0, 40)});
		stencil.drawText(*this->_font2, "Cat!!", {0, 40});
		stencil.popRotation();

		stencil.drawText(*this->_font3, "MeW MeW MeW!", {0, 75});
		// ---

		stencil.popOffset();

		stencil.render();

		// TEST ---
		this->_texture2->step();
		this->_counter += 0.1F;
		// ---
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
		bgfx::dbgTextPrintf(1, 1, 0x1f, "001-stencil");
		bgfx::dbgTextPrintf(1, 2, 0x3f, "Description: 2D Stencil test");
		printFrames();
		// -----------

		if (this->_ready) {
			this->drawOverlay();
		} else {
			bgfx::dbgTextPrintf(1, 10, 0x70, "                                   ");
			bgfx::dbgTextPrintf(1, 11, 0x70, "          LOADING CONTENT          ");
			bgfx::dbgTextPrintf(1, 12, 0x70, "                                   ");
		}

		this->_window->frame(); // Commit primitives
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, nullptr, nullptr);
	}
} // namespace stencil
