
#include <rawrbox/imgui/manager.hpp>
#include <rawrbox/render/cameras/perspective.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/resources/manager.hpp>

#include <imgui/game.hpp>

#include <fmt/format.h>
#include <imgui.h>

namespace imgui {
	void Game::setupGLFW() {
#if defined(_DEBUG) && defined(RAWRBOX_SUPPORT_DX12)
		auto* window = rawrbox::Window::createWindow(Diligent::RENDER_DEVICE_TYPE_D3D12); // DX12 is faster on DEBUG than Vulkan, due to vulkan having extra check steps to prevent you from doing bad things
#else
		auto* window = rawrbox::Window::createWindow();
#endif
		window->setMonitor(-1);
		window->setTitle("IMGUI TEST");
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
		auto* render = window->createRenderer();
		render->onIntroCompleted = [this]() { this->loadContent(); };
		render->setDrawCall([this](const rawrbox::CameraBase& /*camera*/, const rawrbox::DrawPass& pass) {
			if (pass != rawrbox::DrawPass::PASS_OVERLAY) return;
			this->drawOverlay();
		});
		// ---------------

		// Setup camera --
		auto* cam = render->createCamera<rawrbox::CameraPerspective>(render->getSize());
		cam->setPos({-2.F, 5.F, -3.5F});
		cam->setAngle({0.F, rawrbox::MathUtils::toRad(-45), 0.F, 0.F});
		// ---------------

		// Add loaders
		rawrbox::RESOURCES::addLoader<rawrbox::TextureLoader>();
		//  --------------

		render->init();

		// Setup ImGui
		rawrbox::IMGUIManager::init(true);

		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.F, 0.F, 0.F, 0.F);
		// --------------
	}

	void Game::loadContent() {
		std::vector<std::pair<std::string, uint32_t>> initialContentFiles = {
		    {"./assets/textures/meow3.gif", 0},
		};

		rawrbox::RESOURCES::loadListAsync(initialContentFiles, [this]() {
			rawrbox::runOnRenderThread([this]() {
				rawrbox::IMGUIManager::load();

				this->contentLoaded();
			});
		});
	}

	void Game::contentLoaded() {
		if (this->_ready) return;

		this->_texture = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/meow3.gif")->get();
		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_RENDER) {
			rawrbox::RESOURCES::shutdown();
		}

		rawrbox::IMGUIManager::shutdown(thread);
		rawrbox::Window::shutdown(thread);
	}

	void Game::pollEvents() {
		rawrbox::Window::pollEvents();
	}

	void Game::update() {
		rawrbox::Window::update();
	}

	void Game::drawOverlay() {
		if (!this->_ready) return;

		rawrbox::IMGUIManager::clear();
		this->drawIMGUIMenu();
		rawrbox::IMGUIManager::render();
	}

	void Game::drawIMGUIMenu() {
		ImGui::DockSpaceOverViewport(ImGui::GetWindowDockID(), ImGui::GetMainViewport());

		// TEST FRAME --
		ImGui::Begin("Welcome");
		ImGui::Text("Hello Player!");
		ImGui::InputText("Name", this->_inputText.data(), 256);
		ImGui::Image(this->_texture, ImVec2(553, 358));
		ImGui::End();
		// ----------

		// TOOLBAR --
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("Game")) {
				ImGui::Separator();

				if (ImGui::MenuItem("Load")) {
					fmt::print("Load\n");
				}

				if (ImGui::MenuItem("Save")) {
					fmt::print("Save\n");
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Exit")) {
					fmt::print("Exit\n");
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
		// ----------

		ImGui::ShowDemoWindow();
	}

	void Game::draw() {
		rawrbox::Window::render(); // Commit primitives
	}
} // namespace imgui
