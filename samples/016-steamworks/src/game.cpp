
#include <rawrbox/render/cameras/orbital.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/steamworks/sdk.hpp>
#include <rawrbox/steamworks/utils.hpp>

#include <steamworks/game.hpp>

#include <fmt/format.h>

namespace steamworks {
	void Game::setupGLFW() {
#if defined(_DEBUG) && defined(RAWRBOX_SUPPORT_DX12)
		auto* window = rawrbox::Window::createWindow(Diligent::RENDER_DEVICE_TYPE_D3D12); // DX12 is faster on DEBUG than Vulkan, due to vulkan having extra check steps to prevent you from doing bad things
#else
		auto* window = rawrbox::Window::createWindow();
#endif
		window->setMonitor(-1);
		window->setTitle("STEAMWORKS TEST");
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
			if (pass == rawrbox::DrawPass::PASS_OVERLAY) {
				this->drawOverlay();
			} else {
				this->drawWorld();
			}
		});
		// ---------------

		// Setup camera
		auto* cam = render->createCamera<rawrbox::CameraOrbital>(*window);
		cam->setPos({0.F, 5.F, -5.F});
		cam->setAngle({0.F, rawrbox::MathUtils::toRad(-45), 0.F, 0.F});
		// --------------

		if (!rawrbox::SteamSDK::init()) throw std::runtime_error("Failed to start steam sdk");
		render->init();
	}

	void Game::loadContent() {
		CSteamID id = rawrbox::SteamSDK::getSteamID();
		auto plyAvatar = rawrbox::SteamSDK::getAvatar(id, rawrbox::AvatarSize::LARGE);

		if (!plyAvatar.pixels.empty()) {
			this->_avatar = std::make_unique<rawrbox::TextureImage>(rawrbox::Vector2u{plyAvatar.width, plyAvatar.height}, plyAvatar.pixels);
			this->_avatar->setName(std::to_string(id.GetAccountID()));
			this->_avatar->upload();
		}

		// Setup friends -----
		auto friends = rawrbox::SteamSDK::getFriends();

		auto maxFriends = std::min<size_t>(friends.size(), 11U);
		this->_friendAvatars.resize(maxFriends);

		for (size_t i = 0; i < maxFriends; i++) {
			auto avatar = rawrbox::SteamSDK::getAvatar(friends[i], rawrbox::AvatarSize::MEDIUM);
			auto avatarPixels = std::make_unique<rawrbox::TextureImage>(rawrbox::Vector2u{avatar.width, avatar.height}, avatar.pixels);
			avatarPixels->setName(std::to_string(friends[i].GetAccountID()));
			avatarPixels->upload();

			_friendAvatars[i] = std::move(avatarPixels);
		}
		// ------------------

		this->contentLoaded(); // NO CONTENT
	}

	void Game::contentLoaded() {
		if (this->_ready) return;

		// MAIN CUBE ---
		{
			auto mesh = rawrbox::MeshUtils::generateCube<>({0, 0, 0}, {2.F, 2.F, 2.F});
			if (this->_avatar != nullptr) mesh.setTexture(this->_avatar.get());
			this->_model->addMesh(mesh);
		}
		// ----

		// FRIENDS CUBE ---
		{
			for (size_t i = 0; i < this->_friendAvatars.size(); i++) {
				auto& f = this->_friendAvatars[i];
				if (f == nullptr) continue;

				auto pCos = std::cos(static_cast<float>(i) * 0.55F) * 3.5F;
				auto pSin = std::sin(static_cast<float>(i) * 0.55F) * 3.5F;

				auto mesh = rawrbox::MeshUtils::generateCube<>({pSin, pCos, 0}, {1.F, 1.F, 1.F});
				mesh.setTexture(f.get());

				this->_model->addMesh(mesh);
			}
		}
		// ----

		this->_model->upload();
		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_RENDER) {
			this->_avatar.reset();
			this->_friendAvatars.clear();
			this->_model.reset();

			rawrbox::SteamSDK::shutdown();
		}

		rawrbox::Window::shutdown(thread);
	}

	void Game::pollEvents() {
		rawrbox::Window::pollEvents();
	}

	void Game::update() {
		rawrbox::Window::update();
		if (!this->_ready || this->_model == nullptr) return;

		this->_model->setEulerAngle({0, rawrbox::MathUtils::toRad(this->_counter * 20.F), rawrbox::MathUtils::toRad(this->_counter * 10.F)});
		this->_counter += 0.1F;
	}

	void Game::drawOverlay() const {
		if (!this->_ready) return;

		auto* stencil = rawrbox::RENDERER->stencil();
		CSteamID id = rawrbox::SteamSDK::getSteamID();

		stencil->drawText(fmt::format("SteamID: {}", std::to_string(id.GetAccountID())), {2, 25});
	}

	void Game::drawWorld() {
		if (!this->_ready || this->_model == nullptr) return;
		this->_model->draw();
	}

	void Game::draw() {
		rawrbox::Window::render(); // Commit primitives
	}
} // namespace steamworks
