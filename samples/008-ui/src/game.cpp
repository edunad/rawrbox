
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/resources/loaders/json.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/ui/elements/frame.hpp>
#include <rawrbox/ui/elements/group.hpp>
#include <rawrbox/ui/elements/input.hpp>
#include <rawrbox/ui/elements/label.hpp>
#include <rawrbox/ui/elements/progress_bar.hpp>
#include <rawrbox/ui/elements/virtual_list.hpp>
#include <rawrbox/ui/static.hpp>

#include <ui_test/game.hpp>

#include <fmt/format.h>

#include <vector>

namespace ui_test {
	void Game::setupGLFW() {
		this->_window = std::make_unique<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("UI TEST");
		this->_window->setRenderer(
		    bgfx::RendererType::Count, [this]() { if(this->_ROOT_UI == nullptr) return; this->_ROOT_UI->render(); }, []() {});
		this->_window->create(1024, 768, rawrbox::WindowFlags::Debug::TEXT | rawrbox::WindowFlags::Debug::PROFILER | rawrbox::WindowFlags::Window::WINDOWED | rawrbox::WindowFlags::Features::MULTI_THREADED);
		this->_window->onWindowClose += [this](auto& w) { this->shutdown(); };
	}

	void Game::init() {
		if (this->_window == nullptr) return;
		this->_window->initializeBGFX();

		rawrbox::RESOURCES::addLoader<rawrbox::FontLoader>();
		rawrbox::RESOURCES::addLoader<rawrbox::TextureLoader>();

		// SETUP UI
		this->_ROOT_UI = std::make_unique<rawrbox::UIRoot>(*this->_window);
		this->_ROOT_UI->onFocusChange += [](rawrbox::UIContainer* elm) {
			if (elm != nullptr)
				fmt::print("[ROOT_UI] Focusing on element\n");
			else
				fmt::print("[ROOT_UI] Not element to focus\n");
		};
		// ----

		// Load content ---
		this->loadContent();
		// -----
	}

	void Game::loadContent() {
		std::vector initialContentFiles = {
		    std::make_pair<std::string, uint32_t>("cour.ttf", 0),
		    std::make_pair<std::string, uint32_t>("./content/textures/meow3.gif", 0),
		    std::make_pair<std::string, uint32_t>("./content/json/test.json", 0)};
		initialContentFiles.insert(initialContentFiles.begin(), rawrbox::UI_RESOURCES.begin(), rawrbox::UI_RESOURCES.end()); // Insert the UI resources

		for (auto& f : initialContentFiles) {
			this->_loadingFiles++;

			rawrbox::RESOURCES::loadFileAsync(f.first, f.second, [this]() {
				this->_loadingFiles--;
				if (this->_loadingFiles <= 0) {
					rawrbox::runOnRenderThread([this]() { this->contentLoaded(); });
				}
			});
		}
		this->_window->upload();
	}

	void Game::contentLoaded() {
		// SETUP UI
		this->_anim = std::make_unique<rawrbox::UIAnim<rawrbox::UIImage>>();
		this->_anim->setAnimation(*rawrbox::RESOURCES::getFile<rawrbox::ResourceJSON>("./content/json/test.json")->get());

		{
			auto frame = this->_ROOT_UI->createChild<rawrbox::UIFrame>();
			frame->setTitle("mewww");
			frame->setSize({400, 200});
			frame->setPos({600, 200});

			{
				auto label = frame->createChild<rawrbox::UILabel>();
				label->setPos({10, 5});
				label->setFont("cour.ttf", 14);
				label->setText("Label: mew!");
				label->sizeToContents();
			}

			{
				auto label = frame->createChild<rawrbox::UILabel>();
				label->setPos({10, 18});
				label->setFont("cour.ttf", 14);
				label->setText("Label: shadow mew!");
				label->setShadowColor(rawrbox::Colors::Black);
				label->sizeToContents();
			}

			{
				auto input = frame->createChild<rawrbox::UIInput>();
				input->setPos({10, 36});
				input->setSize({380, 22});
				input->setFont("cour.ttf", 14);
				input->setPlaceholder("cour.taaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaatf");
			}

			{
				auto input = frame->createChild<rawrbox::UIInput>();
				input->setPos({10, 64});
				input->setSize({380, 22});
				input->setFont("cour.ttf", 14);
				input->setText("readonly");
				input->setReadOnly(true);
			}

			this->_group = frame->createChild<rawrbox::UIGroup>();
			this->_group->setBorder(1.F);
			this->_group->setPos({215, 96});
			this->_group->setSize({64, 64});

			{
				auto btn = frame->createChild<rawrbox::UIButton>();
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
				auto img = this->_group->createChild<rawrbox::UIImage>();
				img->setTexture("./content/textures/meow3.gif");
				img->setSize({128, 128});
			}

			auto vlist = frame->createChild<rawrbox::UIVirtualList<float>>();
			vlist->setPos({279, 96});
			vlist->setSize({64, 64});
			vlist->renderItem = [](size_t indx, float& msg, bool isHovering, rawrbox::Stencil& stencil) { stencil.drawBox({msg, 0}, {1, 10}); };
			vlist->getItemSize = [](size_t indx) {
				return 9;
			};

			for (size_t i = 0; i < 64; i++)
				vlist->addItem(static_cast<float>(i));

			{
				auto prog = frame->createChild<rawrbox::UIProgressBar>();
				prog->setPos({10, 130});
				prog->setSize({200, 16});
				prog->setValue(50);
				prog->setBarColor(rawrbox::Colors::Orange);
			}

			{
				auto prog = frame->createChild<rawrbox::UIProgressBar>();
				prog->setPos({10, 146});
				prog->setSize({200, 16});
				prog->setValue(100);
			}

			{
				auto img = this->_ROOT_UI->createChild<rawrbox::UIImage>();
				img->setTexture("./content/textures/meow3.gif");
				img->setSize({32, 32});

				this->_anim->setPos({100, 100});
				this->_anim->setElement(img);
				this->_anim->setLoop(true);
				this->_anim->play();
			}
		}

		{
			auto frame = this->_ROOT_UI->createChild<rawrbox::UIFrame>();
			frame->setTitle("graphss");
			frame->setSize({400, 200});
			frame->setPos({100, 200});
			frame->onClose += [this]() {
				this->_graph = nullptr;
			};

			this->_graph = frame->createChild<rawrbox::UIGraph>();
			this->_graph->setPos({10, 10});
			this->_graph->setSize({380, 160});
			this->_graph->addCategory("Cats /s", rawrbox::Colors::Orange);
			this->_graph->addCategory("Meows /s", rawrbox::Colors::Purple);
			this->_graph->setAutoScale(true);
			this->_graph->setSmoothing(20);
		}
		// ---

		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) return;

		this->_anim->reset();
		this->_graph = nullptr;

		this->_ROOT_UI.reset();

		rawrbox::RESOURCES::shutdown();
		rawrbox::ASYNC::shutdown();

		this->_window->unblockPoll();
		this->_window.reset();
	}

	void Game::pollEvents() {
		if (this->_window == nullptr) return;
		this->_window->pollEvents();
	}

	void Game::update() {
		this->_ROOT_UI->update();
		if (this->_anim != nullptr) this->_anim->update();
	}

	void Game::printFrames() {
		const bgfx::Stats* stats = bgfx::getStats();

		double gpu = static_cast<double>(stats->gpuTimeEnd - stats->gpuTimeBegin) * 1000.0 / stats->gpuTimerFreq;
		double cpu = static_cast<double>(stats->cpuTimeEnd - stats->cpuTimeBegin) * 1000.0 / stats->cpuTimerFreq;

		bgfx::dbgTextPrintf(1, 4, 0x6f, "GPU %0.6f [ms]", gpu);
		bgfx::dbgTextPrintf(1, 5, 0x6f, "CPU %0.6f [ms]", cpu);
		bgfx::dbgTextPrintf(1, 6, 0x6f, fmt::format("TRIANGLES: {} ----->    DRAW CALLS: {}", stats->numPrims[bgfx::Topology::TriList], stats->numDraw).c_str());

		if (this->_graph != nullptr) {
			this->_graph->getCategory(0).addEntry(gpu);
			this->_graph->getCategory(1).addEntry(cpu);
		}
	}

	void Game::draw() {
		if (this->_window == nullptr) return;

		// DEBUG ----
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(1, 1, 0x1f, "008-ui");
		bgfx::dbgTextPrintf(1, 2, 0x3f, "Description: UI test");
		printFrames();
		// -----------

		if (!this->_ready) {
			bgfx::dbgTextPrintf(1, 10, 0x70, "                                   ");
			bgfx::dbgTextPrintf(1, 11, 0x70, "          LOADING CONTENT          ");
			bgfx::dbgTextPrintf(1, 12, 0x70, "                                   ");
		}

		this->_window->render(); // Commit primitives
	}
} // namespace ui_test
