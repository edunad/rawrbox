#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/cameras/orbit.hpp>
#include <rawrbox/render/models/instanced.hpp>

#include <limits>
#include <memory>
#include <vector>

namespace instance_test {
	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::InstancedModel<>> _model = std::make_unique<rawrbox::InstancedModel<>>();
		rawrbox::CameraOrbit* _camera = nullptr;
		bool _ready = false;

		// DYNAMIC TEMPLATE ---
		static constexpr size_t DYNAMIC_INSTANCES = 24;

		std::unique_ptr<rawrbox::InstancedModel<>> _dynamic = std::make_unique<rawrbox::InstancedModel<>>();
		std::vector<rawrbox::Instance> _dynamicInstances = {};
		rawrbox::TextureBase* _texture = nullptr;

		float _dynamicTime = 0.F;
		size_t _dynamicShape = std::numeric_limits<size_t>::max();

		void updateDynamic();
		// ---------------

		void setupGLFW() override;
		void init() override;
		void onThreadShutdown(rawrbox::ENGINE_THREADS thread) override;
		void pollEvents() override;
		void update() override;
		void draw() override;

	public:
		Game() = default;
		Game(const Game&) = delete;
		Game(Game&&) = delete;
		Game& operator=(const Game&) = delete;
		Game& operator=(Game&&) = delete;
		~Game() override = default;

		void drawWorld();
		void drawOverlay() const;

		void loadContent();
		void contentLoaded();
	};
} // namespace instance_test