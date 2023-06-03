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
		std::shared_ptr<rawrbox::Model<>> mdl = nullptr;

		BoxOfDoom() = default;
		~BoxOfDoom() {
			body = nullptr;
			mdl.reset();
		}
	};

	class Game : public rawrbox::Engine {
		std::shared_ptr<rawrbox::Window> _window = nullptr;
		std::shared_ptr<rawrbox::CameraOrbital> _camera = nullptr;

		std::shared_ptr<rawrbox::Model<>> _modelGrid = std::make_shared<rawrbox::Model<>>();

		std::vector<BoxOfDoom> _boxes = std::vector<BoxOfDoom>();

		bool _ready = false;
		bool _paused = true;

	public:
		using Engine::Engine;

		void init() override;
		void shutdown() override;
		void pollEvents() override;
		void update() override;
		void draw() override;
		void drawWorld();

		void printFrames();

		void createBox(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& size);

		void loadContent();
		void contentLoaded();
	};
} // namespace physics_test
