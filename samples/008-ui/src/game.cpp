
#include <rawrbox/engine/static.hpp>
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/cameras/perspective.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/window.hpp>
#include <rawrbox/resources/loaders/json.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/ui/elements/button.hpp>
#include <rawrbox/ui/elements/dropdown.hpp>
#include <rawrbox/ui/elements/frame.hpp>
#include <rawrbox/ui/elements/group.hpp>
#include <rawrbox/ui/elements/input.hpp>
#include <rawrbox/ui/elements/label.hpp>
#include <rawrbox/ui/elements/progress_bar.hpp>
#include <rawrbox/ui/elements/tabs.hpp>
#include <rawrbox/ui/elements/virtual_list.hpp>
#include <rawrbox/ui/popup/manager.hpp>
#include <rawrbox/ui/static.hpp>
#include <rawrbox/utils/keys.hpp>

#include <ui_test/game.hpp>

#include <fmt/format.h>

namespace ui_test {

	void Game::setupGLFW() {
#if defined(_DEBUG) && defined(RAWRBOX_SUPPORT_DX12)
		auto* window = rawrbox::Window::createWindow(Diligent::RENDER_DEVICE_TYPE_D3D12); // DX12 is faster on DEBUG than Vulkan, due to vulkan having extra check steps to prevent you from doing bad things
#else
		auto* window = rawrbox::Window::createWindow();
#endif
		window->setMonitor(-1);
		window->setTitle("UI TEST");
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
		render->setDrawCall([this](const rawrbox::DrawPass& pass) {
			if (pass != rawrbox::DrawPass::PASS_OVERLAY) return;
			this->_ROOT_UI->render();
		});
		// ---------------

		// Setup camera --
		auto* cam = render->setupCamera<rawrbox::CameraPerspective>(render->getSize());
		cam->setPos({-2.F, 5.F, -3.5F});
		cam->setAngle({0.F, rawrbox::MathUtils::toRad(-45), 0.F, 0.F});
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

		rawrbox::POPUP::init(this->_ROOT_UI.get());
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
			if (action != rawrbox::KEY_ACTION_DOWN || key != rawrbox::KEY_F1 || this->_consoleUI == nullptr) return;
			this->_consoleUI->setVisible(!this->_consoleUI->visible());
			this->_consoleUI->bringToFront();
		};
		// ----------

		this->_anim = std::make_unique<rawrbox::UIAnim<rawrbox::UIImage>>();
		this->_anim->setAnimation(*rawrbox::RESOURCES::getFile<rawrbox::ResourceJSON>("./assets/json/test.json")->get());

		{
			auto* frame = this->_ROOT_UI->createChild<rawrbox::UIFrame>();
			frame->setTitle("mewww");
			frame->setSize({400, 200});
			frame->setPos({600, 200});
			frame->onClose += [frame]() {
				frame->remove();
			};

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
			frame->onClose += [frame]() {
				frame->remove();
			};

			// LIST
			{
				auto* label = frame->createChild<rawrbox::UILabel>();
				label->setPos({5, 3});
				label->setText("LIST MODE");
				label->sizeToContents();

				auto* vlist = frame->createChild<rawrbox::UIVirtualList<std::string>>();
				vlist->setPos({0, 16});
				vlist->setSize({400, 100});
				vlist->setScrollSpeed(5);
				vlist->renderItem = [](size_t indx, std::string& msg, bool isHovering, rawrbox::Stencil& stencil) {
					stencil.drawBox({}, {400, 12}, isHovering ? rawrbox::Colors::Black() : rawrbox::Colors::Gray().strength(indx % 2 == 1 ? 0.25F : 0.5F));
					stencil.drawText(*rawrbox::DEBUG_FONT_REGULAR, msg, {0, 0});
				};

				vlist->getItemSize = [](size_t /*indx*/) -> rawrbox::Vector2i {
					return {400, 12};
				};

				for (size_t i = 0; i < 64; i++)
					vlist->addItem(fmt::format("Item #{}", i));
			}

			// GRID
			{
				auto* label = frame->createChild<rawrbox::UILabel>();
				label->setPos({5, 120});
				label->setText("GRID MODE");
				label->sizeToContents();

				auto* vlist = frame->createChild<rawrbox::UIVirtualList<std::string>>();
				vlist->setPos({0, 136});
				vlist->setSize({400, 100});
				vlist->setMode(rawrbox::VirtualListMode::GRID);
				vlist->renderItem = [](size_t indx, std::string& msg, bool isHovering, rawrbox::Stencil& stencil) {
					stencil.drawBox({}, {24, 24}, isHovering ? rawrbox::Colors::Black() : rawrbox::Colors::Gray().strength(indx % 2 == 1 ? 0.25F : 0.5F));
					stencil.drawText(*rawrbox::DEBUG_FONT_REGULAR, msg, {0, 0});
				};

				vlist->getItemSize = [](size_t /*indx*/) -> rawrbox::Vector2i {
					return {24, 24};
				};

				for (size_t i = 0; i < 256; i++)
					vlist->addItem(fmt::format("#{}", i));
			}
		}

		{
			auto* frame = this->_ROOT_UI->createChild<rawrbox::UIFrame>();
			frame->setTitle("Graphs");
			frame->setSize({400, 200});
			frame->setPos({100, 200});
			frame->onClose += [this, frame]() {
				this->_graph = nullptr;
				frame->remove();
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

		{
			auto* frame = this->_ROOT_UI->createChild<rawrbox::UIFrame>();
			frame->setTitle("Tabs");
			frame->setSize({300, 250});
			frame->setPos({600, 450});
			frame->onClose += [frame]() {
				frame->remove();
			};

			// TAB 1 ---
			auto* group1 = frame->createChild<rawrbox::UIGroup>();
			auto* label1 = group1->createChild<rawrbox::UILabel>();
			label1->setText("Tab 1");
			label1->sizeToContents();

			auto* button1 = group1->createChild<rawrbox::UIButton>();
			button1->setText("POPUP INFO");
			button1->setEnabled(true);
			button1->setSize({200, 24});
			button1->setPos({10, 40});
			button1->onClick += []() {
				rawrbox::POPUP::create("test-1", "Info", "This is a popup info message");
			};

			auto* button2 = group1->createChild<rawrbox::UIButton>();
			button2->setText("POPUP QUESTION");
			button2->setEnabled(true);
			button2->setSize({200, 24});
			button2->setPos({10, 40 + 26});
			button2->onClick += []() {
				rawrbox::POPUP::create("test-2", "Question", "This is a popup question message", rawrbox::PopupType::QUESTION);
			};

			auto* button3 = group1->createChild<rawrbox::UIButton>();
			button3->setText("POPUP WARNING");
			button3->setEnabled(true);
			button3->setSize({200, 24});
			button3->setPos({10, 40 + 26 * 2});
			button3->onClick += []() {
				rawrbox::POPUP::create("test-3", "Warning", "This is a popup warning message", rawrbox::PopupType::WARNING);
			};

			auto* button4 = group1->createChild<rawrbox::UIButton>();
			button4->setText("POPUP LOADING");
			button4->setEnabled(true);
			button4->setSize({200, 24});
			button4->setPos({10, 40 + 26 * 3});
			button4->onClick += []() {
				rawrbox::POPUP::create("test-4", "Loading", "This is a popup loading message", rawrbox::PopupType::LOADING);
			};

			auto* button5 = group1->createChild<rawrbox::UIButton>();
			button5->setText("POPUP ERROR");
			button5->setEnabled(true);
			button5->setSize({200, 24});
			button5->setPos({10, 40 + 26 * 4});
			button5->onClick += []() {
				rawrbox::POPUP::create("test-5", "Error", "This is a popup error message", rawrbox::PopupType::ERR);
			};

			// --------

			// TAB 2 ---
			auto* group2 = frame->createChild<rawrbox::UIGroup>();
			auto* label2 = group2->createChild<rawrbox::UILabel>();
			label2->setText("Tab 2");
			label2->sizeToContents();
			// --------

			// TAB 3 ---
			auto* group3 = frame->createChild<rawrbox::UIGroup>();
			auto* label3 = group3->createChild<rawrbox::UILabel>();
			label3->setText("Tab 3");
			label3->sizeToContents();
			// --------

			// TAB 4 ---
			auto* group4 = frame->createChild<rawrbox::UIGroup>();
			auto* label4 = group4->createChild<rawrbox::UILabel>();
			label4->setText("Tab 4");
			label4->sizeToContents();
			// --------

			std::vector<rawrbox::UITab> tabs = {
			    {"tab1", "Tab 1", group1},
			    {"tab2", "Tab 2", group2},
			    {"tab3", "Tab 3", group3},
			    {"tab4", "Tab 4", group4}};

			auto* tab = frame->createChild<rawrbox::UITabs>(tabs);
			tab->setSize({300, 250});
			tab->setActive(0);
			tab->setEnabled(2, false);
		}

		{
			auto* frame = this->_ROOT_UI->createChild<rawrbox::UIFrame>();
			frame->setTitle("Dropdown");
			frame->setSize({300, 250});
			frame->setPos({950, 450});
			frame->onClose += [frame]() {
				frame->remove();
			};

			// TODO: FIX RENDERING, TEMP FIX MAKE THE LISTS BEING ADDED REVERSE ORDER
			auto* dropdown2 = frame->createChild<rawrbox::UIDropdown>(std::vector<std::string>{"Longggg option isidfjs sdfssdfsd", "dfgdfgdfg gfhgfh isidfjs sdfssdfsd"});
			dropdown2->setSize({200, 24});
			dropdown2->setPos({10, 50});
			dropdown2->setActive(0);

			auto* dropdown = frame->createChild<rawrbox::UIDropdown>(std::vector<std::string>{"Option 1", "Option 2", "Option 3", "Option 4"});
			dropdown->setSize({200, 24});
			dropdown->setPos({10, 10});
			dropdown->setActive(0);
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

			rawrbox::POPUP::shutdown();
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
