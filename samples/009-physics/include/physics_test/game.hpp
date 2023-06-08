#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/physics/manager.hpp>
#include <rawrbox/render/camera/orbital.hpp>
#include <rawrbox/render/model/model.hpp>
#include <rawrbox/render/window.hpp>

#include <memory>

namespace physics_test {

	struct BoxOfDoom {
	public:
		JPH::Body* body = nullptr;
		std::unique_ptr<rawrbox::Model<>> mdl = nullptr;
	};

	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::Window> _window = nullptr;
		std::unique_ptr<rawrbox::CameraOrbital> _camera = nullptr;

		std::unique_ptr<rawrbox::Model<>> _modelGrid = std::make_unique<rawrbox::Model<>>();

		std::vector<std::unique_ptr<BoxOfDoom>> _boxes = std::vector<std::unique_ptr<BoxOfDoom>>();

		std::atomic<int> _loadingFiles = 0;
		bool _ready = false;
		bool _paused = true;

		void setupGLFW() override;
		void init() override;
		void onThreadShutdown(rawrbox::ENGINE_THREADS thread) override;
		void pollEvents() override;
		void update() override;
		void fixedUpdate() override;
		void draw() override;

	public:
		void drawWorld();
		void printFrames();

		void createBox(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& size);

		void loadContent();
		void contentLoaded();
	};
} // namespace physics_test
