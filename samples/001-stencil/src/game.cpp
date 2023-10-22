
#include <rawrbox/engine/static.hpp>
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/cameras/perspective.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/render/resources/svg.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/resources/manager.hpp>

#include <stencil/game.hpp>

#include <fmt/format.h>

#include <vector>

namespace stencil {
	void Game::setupGLFW() {
		auto window = rawrbox::render::createWindow();
		window->setMonitor(-1);
		window->setTitle("STENCIL TEST");
		window->init(1024, 768, rawrbox::WindowFlags::Window::WINDOWED);
		window->onWindowClose += [this](auto& /*w*/) { this->shutdown(); };
	}

	void Game::init() {
		auto window = rawrbox::render::getWindow();

		// Setup renderer
		auto render = rawrbox::render::createRenderer(window, rawrbox::Color::RGBAHex(0x443355FF));
		render->setOverlayRender([this]() { this->drawOverlay(); });
		render->setWorldRender([this]() { this->drawWorld(); });
		render->onIntroCompleted = [this]() {
			this->loadContent();
		};
		// ---------------

		// Setup camera --
		auto cam = render->setupCamera<rawrbox::CameraPerspective>(render->getSize());
		cam->setPos({-2.F, 5.F, -3.5F});
		cam->setAngle({0.F, rawrbox::MathUtils::toRad(-45), 0.F, 0.F});
		// ---------------

		// Add loaders
		rawrbox::RESOURCES::addLoader<rawrbox::TextureLoader>();
		rawrbox::RESOURCES::addLoader<rawrbox::FontLoader>();
		rawrbox::RESOURCES::addLoader<rawrbox::SVGLoader>();
		//  --------------

		render->init();
	}

	void Game::loadContent() {
		std::array initialContentFiles = {
		    std::make_pair<std::string, uint32_t>("./assets/textures/screem.png", 0),
		    std::make_pair<std::string, uint32_t>("./assets/fonts/droidsans.ttf", 0),
		    std::make_pair<std::string, uint32_t>("./assets/fonts/visitor1.ttf", 0),
		    std::make_pair<std::string, uint32_t>("./assets/fonts/LiberationMono-Regular.ttf", 0),
		    std::make_pair<std::string, uint32_t>("./assets/fonts/LiberationMono-Bold.ttf", 0),
		    std::make_pair<std::string, uint32_t>("./assets/fonts/LiberationMono-Italic.ttf", 0),
		    std::make_pair<std::string, uint32_t>("./assets/textures/meow3.gif", 0),
		    std::make_pair<std::string, uint32_t>("./assets/textures/rawrbox.svg", 0),
		    std::make_pair<std::string, uint32_t>("./assets/textures/cawt.webp", 0),
		    std::make_pair<std::string, uint32_t>("./assets/textures/instance_test.png", 64),
		};

		this->_loadingFiles = static_cast<int>(initialContentFiles.size());
		for (auto& f : initialContentFiles) {
			rawrbox::RESOURCES::loadFileAsync(f.first, f.second, [this]() {
				this->_loadingFiles--;
				if (this->_loadingFiles <= 0) {
					rawrbox::runOnRenderThread([this]() { this->contentLoaded(); });
				}
			});
		}
	}

	void Game::contentLoaded() {
		// Textures ---
		this->_texture = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/screem.png")->get();
		this->_texture2 = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/meow3.gif")->get();
		this->_texture3 = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/instance_test.png")->get();
		this->_texture4 = rawrbox::RESOURCES::getFile<rawrbox::ResourceSVG>("./assets/textures/rawrbox.svg")->get({256, 256});

		this->_texture5 = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/cawt.webp")->get<rawrbox::TextureAnimatedBase>();
		this->_texture5->onEnd += []() {
			fmt::print("end\n");
		};

		this->_font = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>("./assets/fonts/droidsans.ttf")->getSize(28);
		this->_font2 = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>("./assets/fonts/visitor1.ttf")->getSize(18);
		this->_font3 = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>("./assets/fonts/LiberationMono-Regular.ttf")->getSize(12);
		// ----

		// Setup markdown ----
		this->_markdown->fontRegular = this->_font3;
		this->_markdown->fontBold = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>("./assets/fonts/LiberationMono-Bold.ttf")->getSize(12);
		this->_markdown->fontItalic = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>("./assets/fonts/LiberationMono-Italic.ttf")->getSize(12);
		this->_markdown->parse("Markdown test [#ff0000]red color[/], *italic text*, **bold text**, ~corrupt text~");
		// ----

		// Textures ---
		auto mesh = rawrbox::MeshUtils::generateCube({0, 0, 0}, {2.F, 2.F, 2.F});
		mesh.setTexture(this->_texture3);

		std::random_device prng;
		std::uniform_int_distribution<uint16_t> dist(0, 4);
		for (auto& vertice : mesh.vertices) {
			vertice.setAtlasId(dist(prng));
		}

		this->_model->addMesh(mesh);
		this->_model->upload();
		// ----

		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) return;

		this->_texture = nullptr;
		this->_texture2 = nullptr;
		this->_texture3 = nullptr;
		this->_texture4 = nullptr;

		this->_model.reset();

		this->_font = nullptr;
		this->_font2 = nullptr;
		this->_font3 = nullptr;

		this->_markdown.reset();

		rawrbox::RESOURCES::shutdown();
		rawrbox::ASYNC::shutdown();
		rawrbox::render::shutdown();
	}

	void Game::pollEvents() {
		rawrbox::render::pollEvents();
	}

	void Game::drawWorld() {
		if (!this->_ready || this->_model == nullptr) return;
		this->_model->draw();
	}

	void Game::drawOverlay() {
		if (!this->_ready) return;

		auto stencil = rawrbox::render::RENDERER->stencil();
		stencil->pushOffset({20, 50});

		// Box + clipping --
		stencil->pushRotation({this->_counter * 50.5F, {50, 50}});
		stencil->pushClipping({-20, -20, 50, 140});
		stencil->drawBox({0, 0}, {100, 100}, rawrbox::Colors::Green());
		stencil->popClipping();

		stencil->pushClipping({50, -20, 50, 140});
		stencil->drawBox({0, 0}, {100, 100}, rawrbox::Colors::Red());
		stencil->popClipping();
		stencil->popRotation();
		// --

		// Inverted box ---
		stencil->pushOffset({100, 0});
		stencil->pushScale({1.F, -1.F});
		stencil->drawBox({0, 0}, {100, 100}, rawrbox::Colors::Red());
		stencil->popScale();
		stencil->popOffset();
		// ---

		// Outline box ---
		stencil->pushOffset({200, 0});
		stencil->pushOutline({1.F, 2.F});
		stencil->drawBox({0, 0}, {100, 100}, rawrbox::Colors::Purple());
		stencil->popOutline();

		stencil->pushOutline({2.F});
		stencil->drawBox({25, 25}, {50, 50}, rawrbox::Colors::Purple());
		stencil->popOutline();
		stencil->popOffset();
		// ---

		// Triangle ---
		stencil->pushOffset({300, 0});
		stencil->drawTriangle({0, 0}, {0, 0}, rawrbox::Colors::Blue(), {0, 100}, {0, 1}, rawrbox::Colors::Blue(), {100, 0}, {0, 1}, rawrbox::Colors::Blue());
		stencil->popOffset();
		// ---

		// Outline triangle ---
		stencil->pushOffset({400, 0});
		stencil->pushOutline({2.F});
		stencil->drawTriangle({15, 15}, {0, 0}, rawrbox::Colors::Blue(), {15, 65}, {0, 1}, rawrbox::Colors::Blue(), {65, 15}, {0, 1}, rawrbox::Colors::Blue());
		stencil->popOutline();

		stencil->pushOutline({1.F, 1.F});
		stencil->drawTriangle({0, 0}, {0, 0}, rawrbox::Colors::Blue(), {0, 100}, {0, 1}, rawrbox::Colors::Blue(), {100, 0}, {0, 1}, rawrbox::Colors::Blue());
		stencil->popOutline();
		stencil->popOffset();
		// ---

		// Circle ---
		stencil->pushOffset({500, 0});
		stencil->drawCircle({0, 0}, {100, 100}, rawrbox::Colors::Orange(), 16, 0, std::fmod(this->_counter * 50.5F, 360.F));
		stencil->popOffset();
		// ---

		// Outline circle ---
		stencil->pushOffset({600, 0});
		stencil->pushOutline({1.F, 0.25F});
		stencil->drawCircle({0, 0}, {100, 100}, rawrbox::Colors::Red(), 16, 0.F, std::fmod(this->_counter * 50.5F, 360.F));
		stencil->popOutline();

		stencil->pushOutline({2.F});
		stencil->drawCircle({25, 25}, {50, 50}, rawrbox::Colors::Red(), 32, 0.F, std::fmod(this->_counter * 50.5F, 360.F));
		stencil->popOutline();
		stencil->popOffset();
		// ---

		// Line ---
		stencil->pushOffset({700, 0});
		stencil->drawLine({0, 0}, {100, 100}, rawrbox::Colors::Red());
		// ---

		// Outline line ---
		stencil->pushOutline({1.F, 2.F});
		stencil->drawLine({100, 0}, {0, 100}, rawrbox::Colors::Blue());
		stencil->popOutline();

		stencil->pushOutline({3.F, 2.F});
		stencil->drawLine({50, 0}, {50, 100}, rawrbox::Colors::Purple());
		stencil->popOutline();
		stencil->popOffset();
		// ---

		// Texture ---
		stencil->pushOffset({800, 0});
		stencil->drawTexture({0, 0}, {100, 100}, *this->_texture);
		stencil->popOffset();

		stencil->pushOffset({900, 0});
		stencil->drawTexture({0, 0}, {100, 100}, *this->_texture2);
		stencil->popOffset();

		stencil->pushOffset({750, 110});
		stencil->drawTexture({0, 0}, {64, 64}, *this->_texture3, rawrbox::Colors::White(), {}, {1, 1}, static_cast<uint32_t>(this->_counter) % 4);
		stencil->popOffset();

		stencil->pushOffset({820, 110});
		stencil->drawTexture({0, 0}, {509 * 0.35F, 404 * 0.35F}, *this->_texture5);
		stencil->popOffset();
		// ---

		// POLYGON ---
		rawrbox::Polygon poly;
		poly.verts = {
		    {{0, 0}, {0, 0}, rawrbox::Colors::White()},
		    {{0, 100}, {1, 0}, rawrbox::Colors::White()},
		    {{100, 0}, {0, 1}, rawrbox::Colors::White()},
		    {{80, 80}, {1, 1}, rawrbox::Colors::White()}};
		poly.indices = {0, 1, 2,
		    1, 3, 2};

		stencil->pushOffset({0, 270});
		stencil->drawPolygon(poly);

		stencil->pushOutline({1.F, 2.F});
		stencil->pushOffset({140, 0});
		stencil->drawPolygon(poly);
		stencil->popOffset();
		stencil->popOutline();

		stencil->popOffset();
		// -----

		// Z-INDEX TEST ---
		stencil->pushOffset({270, 260});
		stencil->drawBox({0, 0}, {100, 100}, rawrbox::Colors::Purple());
		stencil->drawBox({25, 25}, {50, 50}, rawrbox::Colors::Red());
		stencil->popOffset();
		// -----

		stencil->popOffset();

		// Text ---
		stencil->pushOffset({20, 200});
		stencil->drawText(*this->_font, "Cat ipsum dolor sit amet, steal raw zucchini. $£%&", {});

		auto size = this->_font2->getStringSize("Cat!!");

		stencil->pushRotation({this->_counter * 50.5F, (size / 2.F) + rawrbox::Vector2f(0, 40)});
		stencil->drawText(*this->_font2, "Cat!!", {0, 40});
		stencil->popRotation();

		stencil->drawText(*this->_font3, "MeW MeW MeW! I am a cat, nya.", {0, 75});
		// ---

		// Markdown ---
		_markdown->render(stencil, {0, 90});
		// -----

		stencil->popOffset();

		// SVG ---
		stencil->pushOffset({50, 450});
		stencil->drawTexture({0, 0}, {256, 256}, *this->_texture4);
		stencil->popOffset();
		// -----

		this->_texture5->update();
		this->_texture2->update();

		stencil->render();
	}

	void Game::update() {
		rawrbox::render::update();

		if (this->_ready) {
			if (this->_model != nullptr) {
				this->_model->setEulerAngle({0, rawrbox::MathUtils::toRad(this->_counter * 20.F), rawrbox::MathUtils::toRad(this->_counter * 10.F)});
			}

			this->_counter += 0.1F;
		}
	}

	void Game::draw() {
		rawrbox::render::render(); // Draw world, overlay & commit primitives
	}
} // namespace stencil
