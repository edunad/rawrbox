#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/gltf/model.hpp>
#include <rawrbox/render/cameras/orbit.hpp>
#include <rawrbox/render/lights/base.hpp>
#include <rawrbox/render/materials/lit.hpp>
#include <rawrbox/render/materials/skinnedLit.hpp>
#include <rawrbox/render/plugins/shadows.hpp>
#include <rawrbox/render/post_process/base.hpp>

#include <memory>

namespace light {
	enum class Flicker {
		TV,
		FAILING,
		BUZZ,
		PULSE,
		BLINK
	};

	struct Room {
		std::string_view name;
		rawrbox::Vector3f pos;

		float yaw;
		float pitch;
	};

	struct TestLight {
		rawrbox::LightBase* light = nullptr;
		light::Flicker flicker = light::Flicker::TV;

		float intensity = 1.F;
		float seed = 0.F;

		rawrbox::LightBase* glow = nullptr;
		float glowIntensity = 0.F;

		rawrbox::Mesh<rawrbox::MaterialLit::vertexBufferType>* shade = nullptr;
		float shadeEmission = 0.F;
	};

	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::GLTFModel<rawrbox::MaterialLit>> _house = std::make_unique<rawrbox::GLTFModel<rawrbox::MaterialLit>>();
		std::unique_ptr<rawrbox::GLTFModel<rawrbox::MaterialLit>> _props = std::make_unique<rawrbox::GLTFModel<rawrbox::MaterialLit>>();
		std::unique_ptr<rawrbox::GLTFModel<rawrbox::MaterialSkinnedLit>> _monster = std::make_unique<rawrbox::GLTFModel<rawrbox::MaterialSkinnedLit>>();

		std::vector<light::TestLight> _lights = {};
		rawrbox::CameraOrbit* _camera = nullptr;

		rawrbox::ShadowPlugin* _shadows = nullptr;
		rawrbox::PostProcessBase* _noise = nullptr;

		size_t _room = 0;

		float _time = 0.F;
		bool _ready = false;

		void setupGLFW() override;
		void init() override;
		void onThreadShutdown(rawrbox::ENGINE_THREADS thread) override;
		void pollEvents() override;
		void update() override;
		void draw() override;

		light::TestLight& addLight(rawrbox::LightBase* light, light::Flicker flicker, float intensity, bool shadows = true);

		void setRoom(size_t index);
		[[nodiscard]] float getFlicker(const light::TestLight& light) const;

	public:
		Game() = default;
		Game(const Game&) = delete;
		Game(Game&&) = delete;
		Game& operator=(const Game&) = delete;
		Game& operator=(Game&&) = delete;
		~Game() override = default;

		void loadContent();
		void contentLoaded();

		void drawWorld() const;
		void drawOverlay() const;
	};
} // namespace light