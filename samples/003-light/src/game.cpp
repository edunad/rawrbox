#include <rawrbox/gltf/resources/model.hpp>
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/cameras/orbit.hpp>
#include <rawrbox/render/lights/point.hpp>
#include <rawrbox/render/lights/spot.hpp>
#include <rawrbox/render/plugins/clustered.hpp>
#include <rawrbox/render/plugins/post_process.hpp>
#include <rawrbox/render/post_process/noise.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>

#include <light/game.hpp>
#include <magic_enum/magic_enum.hpp>

#include <array>
#include <string_view>

namespace light {
	constexpr float SCALE = 0.003F;
	static rawrbox::Vector3f scale(const float x, const float y, const float z) {
		return rawrbox::Vector3f{x, y, -z} * SCALE;
	}

	static const std::array<Room, 5> ROOMS = {{
	    {.name = "LIVING ROOM", .pos = scale(-450.F, 520.F, 150.F), .yaw = 90.F, .pitch = -15.F},
	    {.name = "KITCHEN", .pos = scale(-1000.F, 540.F, 700.F), .yaw = -51.F, .pitch = -13.F},
	    {.name = "HALLWAY", .pos = scale(-300.F, 560.F, -620.F), .yaw = -58.F, .pitch = -6.F},
	    {.name = "BATHROOM", .pos = scale(-1100.F, 560.F, -600.F), .yaw = -66.F, .pitch = -16.F},
	    {.name = "BASEMENT", .pos = scale(900.F, -350.F, -450.F), .yaw = -124.F, .pitch = -19.F},
	}};

	static float hash(float n) {
		const float s = std::sin(n * 12.9898F) * 43758.5453F;
		return s - std::floor(s);
	}

	void Game::setupGLFW() {
#if defined(_DEBUG) && defined(RAWRBOX_SUPPORT_DX12)
		auto* window = rawrbox::Window::createWindow(Diligent::RENDER_DEVICE_TYPE_D3D12); // DX12 is faster on DEBUG than Vulkan, due to vulkan having extra check steps to prevent you from doing bad things
#else
		auto* window = rawrbox::Window::createWindow();
#endif
		window->setMonitor(-1);
		window->setTitle("LIGHT + SHADOW TEST");
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
		auto* render = window->createRenderer(rawrbox::Colorf{0.0F, 0.0F, 0.0F, 1.0F});

		render->addPlugin<rawrbox::ClusteredPlugin>();
		this->_shadows = render->addPlugin<rawrbox::ShadowPlugin>();

		auto* postProcess = render->addPlugin<rawrbox::PostProcessPlugin>();
		this->_noise = postProcess->add<rawrbox::PostProcessNoise>(0.08F);

		render->onIntroCompleted = [this]() { this->loadContent(); };
		render->setDrawCall([this](const rawrbox::CameraBase& /*camera*/, const rawrbox::DrawPass& pass) {
			if (pass == rawrbox::DrawPass::PASS_WORLD) {
				this->drawWorld();
			} else {
				this->drawOverlay();
			}
		});
		// ---------------

		// Setup camera
		this->_camera = render->createCamera<rawrbox::CameraOrbit>(*window, 70.F, 0.05F, 60.F);
		this->_camera->setMoveSpeed(4.F);
		this->_camera->canUseKeyboard([]() { return true; });
		this->_camera->canUseMouse([]() { return true; });
		// --------------

		// BINDS ----
		window->onKey += [this](rawrbox::Window& /*w*/, uint32_t key, uint32_t /*scancode*/, uint32_t action, uint32_t /*mods*/) {
			if (!this->_ready || action != rawrbox::KEY_ACTION_UP) return;

			if (key == rawrbox::KEY_F1) this->_shadows->setEnabled(!this->_shadows->isEnabled());
			if (key == rawrbox::KEY_F2) this->_shadows->setLodBias(this->_shadows->getLodBias() >= 4.F ? 0.F : this->_shadows->getLodBias() + 1.F);

			if (key == rawrbox::KEY_F3) rawrbox::LIGHTS::setEnabled(!rawrbox::LIGHTS::isEnabled());

			if (key == rawrbox::KEY_F4) this->_shadows->setDither(this->_shadows->getDither() >= 3.F ? 0.F : this->_shadows->getDither() + 1.F);
#ifdef _DEBUG
			if (key == rawrbox::KEY_F5) this->_shadows->setDebug(static_cast<rawrbox::SHADOW_DEBUG>((static_cast<uint32_t>(this->_shadows->getDebug()) + 1U) % static_cast<uint32_t>(rawrbox::SHADOW_DEBUG::COUNT)));
#endif
			if (key == rawrbox::KEY_F6) this->_shadows->setBudget(this->_shadows->getBudget() >= 64 ? 0 : std::max(this->_shadows->getBudget() * 2U, 8U));

			if (key >= rawrbox::KEY_1 && key < rawrbox::KEY_1 + ROOMS.size()) this->setRoom(key - rawrbox::KEY_1);
		};
		// -----

		this->setRoom(0);

		// Add loaders ----
		rawrbox::RESOURCES::addLoader<rawrbox::FontLoader>();
		rawrbox::RESOURCES::addLoader<rawrbox::GLTFLoader>();
		// -----

		render->init();
	}

	void Game::loadContent() {
		const std::vector<std::pair<std::string, uint32_t>> initialContentFiles = {
		    {"./assets/models/kelley_road/scene.gltf", rawrbox::GLTFLoadFlags::IMPORT_TEXTURES | rawrbox::GLTFLoadFlags::IMPORT_ANIMATIONS}};

		rawrbox::RESOURCES::loadListAsync(initialContentFiles, [this]() {
			rawrbox::runOnRenderThread([this]() {
				this->contentLoaded();
			});
		});
	}

	void Game::contentLoaded() {
		if (this->_ready) return;

		const auto* mdl = rawrbox::RESOURCES::getFile<rawrbox::ResourceGLTF>("./assets/models/kelley_road/scene.gltf")->get();

		// House ----
		this->_house->load(*mdl);
		this->_house->removeMeshIf([](const auto& mesh) { return mesh.skeleton != nullptr || !mesh.isMergeable(); });

		// Lamp glow ---
		auto* hallwayShade = this->_house->getMeshByName("Icosphere.001_accessories_0");
		auto* kitchenShade = this->_house->getMeshByName("Icosphere_accessories_0");

		for (auto* shade : {hallwayShade, kitchenShade}) {
			if (shade == nullptr) continue;

			shade->setMergeable(false);
			shade->textures.emission = shade->textures.texture;
		}
		// ----------

		this->_house->setScale({SCALE, SCALE, SCALE});
		this->_house->upload();
		this->_house->setShadowMode(rawrbox::ShadowMode::STATIC);
		// ----------

		// Props ----
		this->_props->load(*mdl);
		this->_props->removeMeshIf([](const auto& mesh) { return mesh.skeleton != nullptr || mesh.isMergeable(); });
		this->_props->setScale({SCALE, SCALE, SCALE});
		this->_props->upload();
		this->_props->setShadowMode(rawrbox::ShadowMode::DYNAMIC);
		this->_props->playAnimation(true);
		// ----------

		// Monster ----
		this->_monster->load(*mdl);
		this->_monster->removeMeshIf([](const auto& mesh) { return mesh.skeleton == nullptr; });
		this->_monster->setScale({SCALE, SCALE, SCALE});
		this->_monster->upload();
		this->_monster->setShadowMode(rawrbox::ShadowMode::DYNAMIC);
		this->_monster->playAnimation(true);
		// ----------

		// Lights ----
		rawrbox::LIGHTS::setAmbient({0.02F, 0.02F, 0.03F, 1.F});

		this->addLight(rawrbox::LIGHTS::add<rawrbox::SpotLight>(scale(320.F, 250.F, 530.F), rawrbox::Vector3f{0.F, -0.15F, 1.F}.normalized(), rawrbox::Colorf{0.45F, 0.6F, 1.F, 1.F}, 90.F, 150.F, 6.F), light::Flicker::TV, 16.F);
		this->addLight(rawrbox::LIGHTS::add<rawrbox::PointLight>(scale(320.F, 230.F, 500.F), rawrbox::Colorf{0.45F, 0.6F, 1.F, 1.F}, 0.6F), light::Flicker::TV, 4.F, false);

		auto& hallway = this->addLight(rawrbox::LIGHTS::add<rawrbox::SpotLight>(scale(-590.F, 690.F, -850.F), rawrbox::Vector3f{0.F, -1.F, 0.F}, rawrbox::Colorf{1.F, 0.72F, 0.42F, 1.F}, 100.F, 160.F, 4.5F), light::Flicker::FAILING, 10.F);
		hallway.glow = rawrbox::LIGHTS::add<rawrbox::PointLight>(scale(-590.F, 640.F, -850.F), rawrbox::Colorf{1.F, 0.72F, 0.42F, 1.F}, 0.8F);
		hallway.glowIntensity = 3.F;
		hallway.shade = hallwayShade;
		hallway.shadeEmission = 1.5F;

		auto* kitchen = rawrbox::LIGHTS::add<rawrbox::SpotLight>(scale(-1400.F, 690.F, 100.F), rawrbox::Vector3f{0.F, -1.F, 0.F}, rawrbox::Colorf{0.75F, 0.9F, 0.55F, 1.F}, 100.F, 160.F, 5.F);
		kitchen->setShadowLodBias(1.F);

		auto& kitchenLamp = this->addLight(kitchen, light::Flicker::BUZZ, 5.F);
		kitchenLamp.glow = rawrbox::LIGHTS::add<rawrbox::PointLight>(scale(-1400.F, 640.F, 100.F), rawrbox::Colorf{0.75F, 0.9F, 0.55F, 1.F}, 0.8F);
		kitchenLamp.glowIntensity = 3.F;
		kitchenLamp.shade = kitchenShade;
		kitchenLamp.shadeEmission = 1.5F;

		auto* bathroom = rawrbox::LIGHTS::add<rawrbox::PointLight>(scale(-1500.F, 700.F, -830.F), rawrbox::Colorf{0.6F, 0.85F, 0.8F, 1.F}, 3.5F);
		bathroom->setShadowLodBias(1.F);

		this->addLight(bathroom, light::Flicker::FAILING, 4.F);
		this->addLight(rawrbox::LIGHTS::add<rawrbox::PointLight>(scale(200.F, -400.F, 50.F), rawrbox::Colorf{1.F, 0.08F, 0.05F, 1.F}, 6.F), light::Flicker::TV, 14.F);
		// ----------

		this->_ready = true;
	}

	light::TestLight& Game::addLight(rawrbox::LightBase* light, light::Flicker flicker, float intensity, bool shadows) {
		light->setCastShadows(shadows);
		return this->_lights.emplace_back(light, flicker, intensity, static_cast<float>(this->_lights.size()) * 7.31F);
	}

	float Game::getFlicker(const light::TestLight& light) const {
		const float time = this->_time + light.seed;

		switch (light.flicker) {
			case light::Flicker::TV:
				return 0.7F + hash(std::floor(this->_time * 12.F)) * 0.3F;
			case light::Flicker::FAILING:
				if (hash(std::floor(time * 0.6F)) < 0.9F) return 1.F;
				return hash(std::floor(time * 16.F)) > 0.35F ? 1.F : 0.35F;
			case light::Flicker::BUZZ:
				return 0.95F + std::sin(time * 120.F) * 0.03F - (hash(std::floor(time * 3.F)) > 0.96F ? 0.3F : 0.F);
			case light::Flicker::PULSE:
				{
					const float pulse = 0.5F + std::sin(time * 1.4F) * 0.5F;
					return 0.55F + pulse * pulse * 0.45F;
				}
			case light::Flicker::BLINK:
				return std::fmod(time, 1.6F) < 0.12F ? 1.F : 0.F;
		}

		return 1.F;
	}

	void Game::setRoom(const size_t index) {
		const auto& room = ROOMS[index];
		this->_room = index;

		this->_camera->setPos(room.pos);
		this->_camera->setAngle({rawrbox::MathUtils::toRad(room.yaw), rawrbox::MathUtils::toRad(room.pitch), 0.F, 0.F});
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_RENDER) {
			this->_house.reset();
			this->_props.reset();
			this->_monster.reset();

			rawrbox::RESOURCES::shutdown();
		}

		rawrbox::Window::shutdown(thread);
	}

	void Game::pollEvents() {
		rawrbox::Window::pollEvents();
	}

	void Game::update() {
		rawrbox::Window::update();
		if (!this->_ready) return;
		this->_time += rawrbox::FIXED_DELTA_TIME;

		for (const auto& light : this->_lights) {
			const float flicker = this->getFlicker(light);
			light.light->setIntensity(light.intensity * flicker);

			if (light.glow != nullptr) light.glow->setIntensity(light.glowIntensity * flicker);
			if (light.shade != nullptr) light.shade->textures.emissionFactor = light.shadeEmission * flicker;
		}
	}

	void Game::drawWorld() const {
		if (!this->_ready) return;

		this->_house->draw();
		this->_props->draw();
		this->_monster->draw();
	}

	void Game::drawOverlay() const {
		if (!this->_ready || this->_shadows == nullptr) return;

		auto* stencil = rawrbox::RENDERER->stencil();

		stencil->drawText(fmt::format("[F1]   SHADOWS: {}", this->_shadows->isEnabled()), {15, 15});
		stencil->drawText(fmt::format("[F2]   LOD BIAS: {}", this->_shadows->getLodBias()), {15, 30});
		stencil->drawText(fmt::format("[F3]   FULLBRIGHT: {}", !rawrbox::LIGHTS::isEnabled()), {15, 45});
		stencil->drawText(fmt::format("[F4]   DITHER: {}", this->_shadows->getDither()), {15, 60});
#ifdef _DEBUG
		stencil->drawText(fmt::format("[F5]   SHADOW DEBUG: {}", magic_enum::enum_name(this->_shadows->getDebug())), {15, 90});
#endif
		stencil->drawText(fmt::format("[F6]   RENDER BUDGET: {}", this->_shadows->getBudget() == 0 ? "unlimited" : fmt::format("{} pages", this->_shadows->getBudget())), {15, 105});

		stencil->drawText(fmt::format("[1-{}]  ROOM: {}", ROOMS.size(), ROOMS[this->_room].name), {15, 135});
	}

	void Game::draw() {
		rawrbox::Window::render(); // Commit primitives
	}
} // namespace light