
#include <rawrbox/engine/static.hpp>
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/cameras/perspective.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/render/resources/svg.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/timer.hpp>

#include <stencil/game.hpp>

#include <fmt/format.h>

#include <vector>

namespace stencil {
	void Game::setupGLFW() {
#if defined(_DEBUG) && defined(RAWRBOX_SUPPORT_DX12)
		auto* window = rawrbox::Window::createWindow(Diligent::RENDER_DEVICE_TYPE_D3D12); // DX12 is faster on DEBUG than Vulkan, due to vulkan having extra check steps to prevent you from doing bad things
#else
		auto* window = rawrbox::Window::createWindow();
#endif
		window->setMonitor(-1);
		window->setTitle("STENCIL TEST");
#ifdef _DEBUG
		window->init(1600, 900, rawrbox::WindowFlags::Window::WINDOWED);
#else
		window->init(0, 0, rawrbox::WindowFlags::Window::BORDERLESS);
#endif
		window->onWindowClose += [this](auto& /*w*/) { this->shutdown(); };
	}

	void Game::init() {
		auto* window = rawrbox::Window::getWindow();

		// Setup renderer
		auto* render = window->createRenderer(rawrbox::Color::RGBAHex(0x443355FF));
		render->onIntroCompleted = [this]() { this->loadContent(); };
		render->setDrawCall([this](const rawrbox::CameraBase& /*camera*/, const rawrbox::DrawPass& pass) {
			if (pass == rawrbox::DrawPass::PASS_OVERLAY) {
				this->drawOverlay();
			} else if (pass == rawrbox::DrawPass::PASS_WORLD) {
				this->drawWorld();
			}
		});
		// ---------------

		// Setup camera --
		auto* cam = render->createCamera<rawrbox::CameraPerspective>(render->getSize());
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
		std::vector<std::pair<std::string, uint32_t>> initialContentFiles = {
		    {"consolab.ttf", 0},
		    {"consolai.ttf", 0},
		    {"./assets/textures/screem.png", 0},
		    {"./assets/fonts/droidsans.ttf", 0},
		    {"./assets/fonts/visitor1.ttf", 0},
		    {"./assets/textures/meow3.gif", 0},
		    {"./assets/textures/rawrbox.svg", 0},
		    {"./assets/textures/cawt.webp", 0},
		    {"./assets/textures/instance_test.png", 64},
		};

		rawrbox::RESOURCES::loadListAsync(initialContentFiles, [this]() {
			rawrbox::runOnRenderThread([this]() {
				this->contentLoaded();
			});
		});
	}

	void Game::contentLoaded() {
		if (this->_ready) return;

		// Textures ---
		this->_texture = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/screem.png")->get();
		this->_texture2 = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/meow3.gif")->get();
		this->_texture3 = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/instance_test.png")->get();
		this->_texture4 = rawrbox::RESOURCES::getFile<rawrbox::ResourceSVG>("./assets/textures/rawrbox.svg")->get({256, 256});

		this->_texture5 = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/cawt.webp")->get<rawrbox::TextureAnimatedBase>();
		this->_texture5->onEnd += []() {
			fmt::print("end\n");
		};

		this->_textureMissing = std::make_unique<rawrbox::TextureMissing>(rawrbox::Vector2u(64, 64));
		this->_textureMissing->upload();

		this->_textureStreaming = std::make_unique<rawrbox::TextureStreaming>(this->_texture->getSize());
		this->_textureStreaming->upload();

		// Simulate loading --
		rawrbox::TIMER::simple(4000.F, [this]() {
			rawrbox::ImageData data = {};
			data.size = this->_texture->getSize();
			data.channels = this->_texture->getChannels();
			data.createFrame(this->_texture->getPixels());

			this->_textureStreaming->setImage(data);
		});
		// ----

		this->_font = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>("./assets/fonts/droidsans.ttf")->getSize(28);
		this->_font2 = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>("./assets/fonts/visitor1.ttf")->getSize(18);
		// ----

		// Setup markdown ----
		this->_markdown->fontRegular = rawrbox::DEBUG_FONT_REGULAR;
		this->_markdown->fontBold = rawrbox::DEBUG_FONT_BOLD;
		this->_markdown->fontItalic = rawrbox::DEBUG_FONT_ITALIC;
		this->_markdown->parse("Markdown test [#ff0000]red color[/], *italic text*, **bold text**, ~corrupt text~");
		// ----

		// Textures ---
		auto mesh = rawrbox::MeshUtils::generateCube({0, 0, 0}, {2.F, 2.F, 2.F});
		mesh.setTexture(this->_texture3);

		std::random_device prng;
		std::uniform_int_distribution<uint16_t> dist(0, 4);
		for (auto& vertice : mesh.vertices) {
			vertice.setSlice(dist(prng));
		}

		this->_model->addMesh(mesh);
		this->_model->upload();
		// ----

		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_RENDER) {
			this->_texture = nullptr;
			this->_texture2 = nullptr;
			this->_texture3 = nullptr;
			this->_texture4 = nullptr;
			this->_texture5 = nullptr;
			this->_textureMissing.reset();
			this->_textureStreaming.reset();

			this->_model.reset();

			this->_font = nullptr;
			this->_font2 = nullptr;

			this->_markdown.reset();

			rawrbox::RESOURCES::shutdown();
		}

		rawrbox::Window::shutdown(thread);
	}

	void Game::pollEvents() {
		rawrbox::Window::pollEvents();
	}

	void Game::drawWorld() {
		if (!this->_ready || this->_model == nullptr) return;
		this->_model->draw();
	}

	void Game::drawOverlay() {
		if (!this->_ready) return;

		auto* stencil = rawrbox::RENDERER->stencil();

		stencil->pushOffset({20, 50});

		// Box + clipping --
		stencil->pushRotation({this->_counter * 50.5F, {50, 50}});
		stencil->pushClipping({rawrbox::AABBu{0, 0, 50, 140}});
		stencil->drawBox({0, 0}, {100, 100}, rawrbox::Colors::Green());
		stencil->popClipping();

		stencil->pushClipping({rawrbox::AABBu{50, 0, 50, 140}});
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

		stencil->pushOffset({910, 0});
		stencil->drawTexture({0, 0}, {100, 100}, *this->_texture2);
		stencil->popOffset();

		stencil->pushOffset({1020, 0});
		stencil->drawTexture({0, 0}, {100, 100}, *this->_texture3, rawrbox::Colors::White(), {}, {1, 1}, static_cast<uint32_t>(this->_counter) % 4);
		stencil->popOffset();

		stencil->pushOffset({1130, 0});
		stencil->drawTexture({0, 0}, {509 * 0.35F, 404 * 0.35F}, *this->_texture5);
		stencil->popOffset();

		stencil->pushOffset({1320, 0});
		stencil->drawTexture({0, 0}, {100, 100}, *this->_textureMissing);
		stencil->popOffset();

		stencil->pushOffset({1430, 0});

		if (this->_textureStreaming->hasData()) {
			stencil->drawTexture({0, 0}, {100, 100}, *this->_textureStreaming);
		} else {
			stencil->drawLoading({0, 0}, {100, 100});
		}

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

		stencil->drawText(*rawrbox::DEBUG_FONT_BOLD, "MeW MeW MeW! I am a cat, nya.", {0, 75});
		stencil->drawText("Quick dirty text", {0, 107});
		// ---

		// Markdown ---
		_markdown->render(*stencil, {0, 90});
		// -----

		stencil->popOffset();

		// SVG ---
		stencil->pushOffset({50, 450});
		stencil->drawTexture({0, 0}, {256, 256}, *this->_texture4);
		stencil->popOffset();
		// -----
	}

	void Game::update() {
		rawrbox::Window::update();
		if (!this->_ready || this->_model == nullptr) return;

		this->_model->setEulerAngle({0, rawrbox::MathUtils::toRad(this->_counter * 20.F), rawrbox::MathUtils::toRad(this->_counter * 10.F)});
		this->_counter += 0.1F;
	}

	void Game::draw() {
		rawrbox::Window::render(); // Draw world, overlay & commit primitives
	}
} // namespace stencil
