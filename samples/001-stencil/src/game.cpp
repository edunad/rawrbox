
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/render/resources/gif.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/resources/static.hpp>

#include <stencil/game.hpp>

#include <bx/bx.h>
#include <bx/math.h>
#include <fmt/format.h>

#include <vector>

namespace stencil {
	void Game::init() {
		int width = 1024;
		int height = 768;

		this->_window = std::make_shared<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("STENCIL TEST");
		this->_window->setClearColor(0x443355FF);
		this->_window->setRenderer(bgfx::RendererType::Count);
		this->_window->initialize(width, height, rawrbox::WindowFlags::Debug::TEXT | rawrbox::WindowFlags::Debug::PROFILER | rawrbox::WindowFlags::Window::WINDOWED);
		this->_window->onWindowClose += [this](auto& w) {
			this->shutdown();
		};

		rawrbox::Resources.addLoader(std::make_unique<rawrbox::TextureLoader>());
		rawrbox::Resources.addLoader(std::make_unique<rawrbox::GIFLoader>());
		rawrbox::Resources.addLoader(std::make_unique<rawrbox::FontLoader>());

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
		// Textures ---
		this->_texture = rawrbox::Resources.getFile<rawrbox::ResourceTexture>("./content/textures/screem.png")->texture;
		this->_texture2 = rawrbox::Resources.getFile<rawrbox::ResourceGIF>("./content/textures/meow3.gif")->texture;

		this->_font = rawrbox::Resources.getFile<rawrbox::ResourceFont>("./content/fonts/droidsans.ttf")->getSize(28);
		this->_font2 = rawrbox::Resources.getFile<rawrbox::ResourceFont>("./content/fonts/visitor1.ttf")->getSize(18);
		this->_font3 = rawrbox::Resources.getFile<rawrbox::ResourceFont>("cour.ttf")->getSize(12);

		this->_ready = true;
	}

	void Game::shutdown() {
		this->_window = nullptr;
		this->_texture = nullptr;
		this->_texture2 = nullptr;

		this->_font.reset();
		this->_font2.reset();
		this->_font3.reset();

		rawrbox::Resources.shutdown();
		rawrbox::Engine::shutdown();
	}

	void Game::pollEvents() {
		if (this->_window == nullptr) return;
		this->_window->pollEvents();
	}

	void Game::drawOverlay() {
		auto& stencil = this->_window->getStencil();

		stencil.begin();

		stencil.pushOffset({20, 50});
		stencil.pushRotation({counter * 50.5F, {50, 50}});
		stencil.drawBox({0, 0}, {100, 100}, rawrbox::Colors::Green);
		stencil.popRotation();

		stencil.pushOffset({100, 0});
		stencil.pushScale({1.F, -1.F});
		stencil.drawBox({0, 0}, {100, 100}, rawrbox::Colors::Red);
		stencil.popScale();
		stencil.popOffset();

		stencil.pushOffset({200, 0});
		stencil.pushOutline({1.F, 2.F});
		stencil.drawBox({0, 0}, {100, 100}, rawrbox::Colors::Purple);
		stencil.popOutline();

		stencil.pushOutline({2.F});
		stencil.drawBox({25, 25}, {50, 50}, rawrbox::Colors::Purple);
		stencil.popOutline();
		stencil.popOffset();

		stencil.pushOffset({300, 0});
		stencil.drawTriangle({0, 0}, {0, 0}, rawrbox::Colors::Blue, {0, 100}, {0, 1}, rawrbox::Colors::Blue, {100, 0}, {0, 1}, rawrbox::Colors::Blue);
		stencil.popOffset();

		stencil.pushOffset({400, 0});
		stencil.pushOutline({2.F});
		stencil.drawTriangle({15, 15}, {0, 0}, rawrbox::Colors::Blue, {15, 65}, {0, 1}, rawrbox::Colors::Blue, {65, 15}, {0, 1}, rawrbox::Colors::Blue);
		stencil.popOutline();

		stencil.pushOutline({1.F, 1.F});
		stencil.drawTriangle({0, 0}, {0, 0}, rawrbox::Colors::Blue, {0, 100}, {0, 1}, rawrbox::Colors::Blue, {100, 0}, {0, 1}, rawrbox::Colors::Blue);
		stencil.popOutline();
		stencil.popOffset();

		stencil.pushOffset({500, 0});
		stencil.drawCircle({0, 0}, {100, 100}, rawrbox::Colors::Orange, 16, 0, std::fmod(counter * 50.5F, 360.F));
		stencil.popOffset();

		stencil.pushOffset({600, 0});
		stencil.pushOutline({1.F, 0.25F});
		stencil.drawCircle({0, 0}, {100, 100}, rawrbox::Colors::Red, 16, 0.F, std::fmod(counter * 50.5F, 360.F));
		stencil.popOutline();

		stencil.pushOutline({2.F});
		stencil.drawCircle({25, 25}, {50, 50}, rawrbox::Colors::Red, 16, 0.F, std::fmod(counter * 50.5F, 360.F));
		stencil.popOutline();
		stencil.popOffset();

		stencil.pushOffset({700, 0});
		stencil.drawLine({0, 0}, {100, 100}, rawrbox::Colors::Red);

		stencil.pushOutline({1.F, 2.F});
		stencil.drawLine({100, 0}, {0, 100}, rawrbox::Colors::Blue);
		stencil.popOutline();

		stencil.pushOutline({3.F, 2.F});
		stencil.drawLine({50, 0}, {50, 100}, rawrbox::Colors::Purple);
		stencil.popOutline();
		stencil.popOffset();

		stencil.pushOffset({800, 0});
		stencil.drawTexture({0, 0}, {100, 100}, this->_texture);
		stencil.popOffset();

		stencil.pushOffset({900, 0});
		stencil.drawTexture({0, 0}, {100, 100}, this->_texture2);
		stencil.popOffset();
		stencil.popOffset();

		stencil.pushOffset({20, 200});
		stencil.drawText(this->_font, "Cat ipsum dolor sit amet, steal raw zucchini off kitchen counter. $£%&", {});

		auto size = this->_font2.lock()->getStringSize("Cat!!");
		stencil.pushRotation({counter * 50.5F, (size / 2.F) + rawrbox::Vector2f(0, 40)});
		stencil.drawText(this->_font2, "Cat!!", {0, 40});
		stencil.popRotation();

		stencil.drawText(this->_font3, "MeW MeW MeW!", {0, 75});
		stencil.popOffset();

		stencil.end();

		// TEST ---
		this->_texture2->step();
		counter += 0.1F;
		// ---
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
