
#include <rawrbox/engine/static.hpp>
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/window.hpp>
#include <rawrbox/resources/loaders/json.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/ui/elements/frame.hpp>
#include <rawrbox/ui/elements/group.hpp>
#include <rawrbox/ui/elements/input.hpp>
#include <rawrbox/ui/elements/label.hpp>
#include <rawrbox/ui/elements/progress_bar.hpp>
#include <rawrbox/ui/elements/virtual_list.hpp>
#include <rawrbox/ui/static.hpp>
#include <rawrbox/utils/keys.hpp>

#include <ui_test/game.hpp>

#include <fmt/format.h>

namespace ui_test {

	void Game::setupGLFW() {
#if defined(_DEBUG) && defined(RAWRBOX_SUPPORT_DX12)
		auto* window = rawrbox::Window::createWindow(Diligent::RENDER_DEVICE_TYPE_D3D12); // DX12 is faster on DEBUG than Vulkan, due to vulkan having extra check steps to prevent you from doing bad things
#else
		auto window = rawrbox::Window::createWindow();
#endif
		window->setMonitor(-1);
		window->setTitle("UI TEST");
#ifdef _DEBUG
		window->init(1024, 768, rawrbox::WindowFlags::Window::WINDOWED);
#else
		window->init(1024, 768, rawrbox::WindowFlags::Window::FULLSCREEN);
#endif
		window->onWindowClose += [this](auto& /*w*/) { this->shutdown(); };
	}

	void Game::init() {
		auto* window = rawrbox::Window::getWindow();

		// Setup renderer
		auto* render = window->createRenderer(rawrbox::Color::RGBAHex(0x443355FF));
		render->onIntroCompleted = [this]() { this->loadContent(); };
		render->setDrawCall([this](const rawrbox::DrawPass& pass) {
			if (pass != rawrbox::DrawPass::PASS_OVERLAY) return;
			this->_ROOT_UI->render();
		});
		// ---------------

		// Add loaders
		rawrbox::RESOURCES::addLoader<rawrbox::TextureLoader>();
		rawrbox::RESOURCES::addLoader<rawrbox::FontLoader>();
		rawrbox::RESOURCES::addLoader<rawrbox::JSONLoader>();
		//  --------------

		// SETUP UI
		this->_ROOT_UI = std::make_unique<rawrbox::UIRoot>(*window);
		this->_ROOT_UI->onFocusChange += [](rawrbox::UIContainer* elm) {
			if (elm != nullptr)
				fmt::print("[ROOT_UI] Focusing on element\n");
			else
				fmt::print("[ROOT_UI] No element to focus\n");
		};
		// ----

		this->_console = std::make_unique<rawrbox::Console>();
		this->_console->registerCommand(
		    "test", [](const std::vector<std::string>& /*args*/) { return std::make_pair<bool, std::string>(true, "OK!"); }, "TEST DESCRIPTION", rawrbox::ConsoleFlags::CHEAT);

		render->init();
	}

	void Game::loadContent() {
		std::vector<std::pair<std::string, uint32_t>> initialContentFiles = {
		    {"./assets/textures/meow3.gif", 0},
		    {"./assets/json/test.json", 0},
		};

		initialContentFiles.insert(initialContentFiles.begin(), rawrbox::UI_RESOURCES.begin(), rawrbox::UI_RESOURCES.end()); // Insert the UI resources
		rawrbox::RESOURCES::loadListAsync(initialContentFiles, [this]() {
			rawrbox::runOnRenderThread([this]() {
				rawrbox::BindlessManager::processBarriers(); // IMPORTANT: BARRIERS NEED TO BE PROCESSED AFTER LOADING ALL THE CONTENT
				this->contentLoaded();
			});
		});
	}

	void Game::contentLoaded() {
		if (this->_ready) return;

		// Setup binds ---
		auto* window = rawrbox::Window::getWindow();
		auto winSize = window->getSize().cast<float>();

		this->_consoleUI = this->_ROOT_UI->createChild<rawrbox::UIConsole>(*this->_console);
		this->_consoleUI->setAlwaysTop(true);
		this->_consoleUI->setPos({0, winSize.y - 240});
		this->_consoleUI->setSize({winSize.x, 240});
		this->_consoleUI->setVisible(false);

		window->onKey += [this](rawrbox::Window& /*w*/, uint32_t key, uint32_t /*scancode*/, uint32_t action, uint32_t /*mods*/) {
			if (action != KEY_ACTION_UP || key != KEY_F1 || this->_consoleUI == nullptr) return;
			this->_consoleUI->setVisible(!this->_consoleUI->visible());
		};
		// ----------

		this->_anim = std::make_unique<rawrbox::UIAnim<rawrbox::UIImage>>();
		this->_anim->setAnimation(*rawrbox::RESOURCES::getFile<rawrbox::ResourceJSON>("./assets/json/test.json")->get());

		{
			auto* frame = this->_ROOT_UI->createChild<rawrbox::UIFrame>();
			frame->setTitle("mewww");
			frame->setSize({400, 200});
			frame->setPos({600, 200});

			{
				auto* label = frame->createChild<rawrbox::UILabel>();
				label->setPos({10, 5});
				label->setText("Label: mew!");
				label->sizeToContents();
			}

			{
				auto* label = frame->createChild<rawrbox::UILabel>();
				label->setPos({10, 18});
				label->setText("Label: shadow mew!");
				label->setShadowColor(rawrbox::Colors::Black());
				label->sizeToContents();
			}

			{
				auto* input = frame->createChild<rawrbox::UIInput>();
				input->setPos({10, 36});
				input->setSize({380, 22});
				input->setPlaceholder("placeholder");
			}

			{
				auto* input = frame->createChild<rawrbox::UIInput>();
				input->setPos({10, 64});
				input->setSize({380, 22});
				input->setText("readonly");
				input->setReadOnly(true);
			}

			this->_group = frame->createChild<rawrbox::UIGroup>();
			this->_group->setBorder(1.F);
			this->_group->setPos({215, 96});
			this->_group->setSize({64, 64});

			{
				auto* btn = frame->createChild<rawrbox::UIButton>();
				btn->setPos({10, 96});
				btn->setSize({200, 32});
				btn->setText("MEW");
				btn->setEnabled(true);
				btn->onClick += [this]() {
					if (this->_group == nullptr) return;

					this->_group->remove();
					this->_group = nullptr;
				};
			}

			{
				auto* img = this->_group->createChild<rawrbox::UIImage>();
				img->setTexture("./assets/textures/meow3.gif");
				img->setSize({128, 128});
			}

			{
				auto* prog = frame->createChild<rawrbox::UIProgressBar>();
				prog->setPos({10, 130});
				prog->setSize({200, 16});
				prog->setValue(50);
				prog->setBarColor(rawrbox::Colors::Orange());
			}

			{
				auto* prog = frame->createChild<rawrbox::UIProgressBar>();
				prog->setPos({10, 146});
				prog->setSize({200, 16});
				prog->setValue(100);
			}

			{
				auto* img = this->_ROOT_UI->createChild<rawrbox::UIImage>();
				img->setTexture("./assets/textures/meow3.gif");
				img->setSize({32, 32});

				this->_anim->setPos({100, 100});
				this->_anim->setElement(img);
				this->_anim->setLoop(true);
				this->_anim->play();
			}

			{
				auto* btn = frame->createChild<rawrbox::UIButton>();
				btn->setPos({290, 96});
				btn->setSize({100, 32});
				btn->setText("LINES");
				btn->setEnabled(true);
				btn->onClick += [this]() {
					if (this->_graph == nullptr) return;
					this->_graph->setStyle(rawrbox::UIGraphStyle::LINE);
				};
			}

			{
				auto* btn = frame->createChild<rawrbox::UIButton>();
				btn->setPos({290, 140});
				btn->setSize({100, 32});
				btn->setText("BLOCK");
				btn->setEnabled(true);
				btn->onClick += [this]() {
					if (this->_graph == nullptr) return;
					this->_graph->setStyle(rawrbox::UIGraphStyle::BLOCK);
				};
			}
		}

		{
			auto* frame = this->_ROOT_UI->createChild<rawrbox::UIFrame>();
			frame->setTitle("Virtual lists");
			frame->setSize({400, 250});
			frame->setPos({100, 450});
			{
				auto* label = frame->createChild<rawrbox::UILabel>();
				label->setPos({5, 3});
				label->setText("LIST MODE");
				label->sizeToContents();
			}

			// LIST
			{
				auto* vlist = frame->createChild<rawrbox::UIVirtualList<std::string>>();
				vlist->setPos({0, 16});
				vlist->setSize({400, 100});
				vlist->renderItem = [](size_t indx, std::string& msg, bool isHovering, rawrbox::Stencil& stencil) {
					stencil.drawBox({}, {400, 12}, isHovering ? rawrbox::Colors::Black() : rawrbox::Colors::Gray().strength(indx % 2 == 1 ? 0.25F : 0.5F));
					stencil.drawText(*rawrbox::DEBUG_FONT_REGULAR, msg, {0, 0});
				};

				vlist->getItemSize = [](size_t /*indx*/) {
					return rawrbox::Vector2i(400, 12);
				};

				for (size_t i = 0; i < 64; i++)
					vlist->addItem(fmt::format("Item #{}", i));
			}

			{
				auto* label = frame->createChild<rawrbox::UILabel>();
				label->setPos({5, 120});
				label->setText("GRID MODE");
				label->sizeToContents();
			}
			// GRID
			{
				auto* vlist = frame->createChild<rawrbox::UIVirtualList<std::string>>();
				vlist->setPos({0, 136});
				vlist->setSize({400, 100});
				vlist->setMode(rawrbox::VirtualListMode::GRID);
				vlist->renderItem = [](size_t indx, std::string& msg, bool isHovering, rawrbox::Stencil& stencil) {
					stencil.drawBox({}, {24, 24}, isHovering ? rawrbox::Colors::Black() : rawrbox::Colors::Gray().strength(indx % 2 == 1 ? 0.25F : 0.5F));
					stencil.drawText(*rawrbox::DEBUG_FONT_REGULAR, msg, {0, 0});
				};

				vlist->getItemSize = [](size_t /*indx*/) {
					return rawrbox::Vector2i(24, 24);
				};

				for (size_t i = 0; i < 256; i++)
					vlist->addItem(fmt::format("#{}", i));
			}
		}

		{
			auto* frame = this->_ROOT_UI->createChild<rawrbox::UIFrame>();
			frame->setTitle("graphss");
			frame->setSize({400, 200});
			frame->setPos({100, 200});
			frame->onClose += [this]() {
				this->_graph = nullptr;
			};

			this->_graph = frame->createChild<rawrbox::UIGraph>();
			this->_graph->setPos({10, 10});
			this->_graph->setSize({380, 160});

#ifdef _DEBUG
			this->_graph->addCategory("DURATION", rawrbox::Colors::Orange());
			this->_graph->addCategory("PRIMITIVES", rawrbox::Colors::Purple());
			this->_graph->addCategory("VERTICES", rawrbox::Colors::Green());
#else
			this->_graph->addCategory("DELTA TIME", rawrbox::Colors::Orange());
			this->_graph->addCategory("FIXED DELTA TIME", rawrbox::Colors::Purple());
#endif
			this->_graph->setAutoScale(true);
			this->_graph->setSmoothing(20);
			this->_graph->setShowLegend(true);
		}
		// ---

		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) {
			rawrbox::Window::shutdown();
		} else {
			this->_anim->reset();
			this->_graph = nullptr;
			this->_consoleUI = nullptr;

			this->_ROOT_UI.reset();
			this->_console.reset();

			rawrbox::RESOURCES::shutdown();
		}
	}

	void Game::pollEvents() {
		rawrbox::Window::pollEvents();
	}

	void Game::update() {
		rawrbox::Window::update();

		if (this->_ready) {
			if (this->_ROOT_UI != nullptr) this->_ROOT_UI->update();
			if (this->_anim != nullptr) this->_anim->update();

			if (this->_graph != nullptr) {
#ifdef _DEBUG
				/*auto& pipelineStats = rawrbox::RENDERER->getPipelineStats("OVERLAY");
				auto& durationStats = rawrbox::RENDERER->getDurationStats("OVERLAY");

				if (durationStats.Frequency > 0) {
					this->_graph->getCategory(0).addEntry(static_cast<float>(durationStats.Duration) / static_cast<float>(durationStats.Frequency) * 1000000.F);
				}

				this->_graph->getCategory(1).addEntry(static_cast<float>(pipelineStats.InputPrimitives));
				this->_graph->getCategory(2).addEntry(static_cast<float>(pipelineStats.InputVertices));*/
#else
				this->_graph->getCategory(0).addEntry(rawrbox::DELTA_TIME);
#endif
				this->_graph->getCategory(0).addEntry(rawrbox::DELTA_TIME);
			}
		}
	}

	void Game::fixedUpdate() {
		if (!this->_ready) return;

#ifndef _DEBUG
		if (this->_graph != nullptr) {
			this->_graph->getCategory(1).addEntry(rawrbox::FIXED_DELTA_TIME);
		}
#endif
	}

	void Game::draw() {
		rawrbox::Window::render(); // Draw world, overlay & commit primitives
	}
} // namespace ui_test
