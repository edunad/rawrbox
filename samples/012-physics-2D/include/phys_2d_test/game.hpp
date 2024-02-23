#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/models/model.hpp>
#include <rawrbox/utils/timer.hpp>

#include <muli/muli.h>

#include <memory>

namespace phys_2d_test {
	struct BoxOfDoom {
	public:
		muli::RigidBody* body = nullptr;
		std::unique_ptr<rawrbox::Model<>> mdl = nullptr;
	};

	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::Model<>> _modelGrid = std::make_unique<rawrbox::Model<>>();
		std::vector<std::unique_ptr<BoxOfDoom>> _boxes = std::vector<std::unique_ptr<BoxOfDoom>>();

		rawrbox::TextureBase* _texture = nullptr;
		rawrbox::TIMER* _timer = nullptr;

		bool _paused = true;
		bool _ready = false;

		void setupGLFW() override;
		void init() override;
		void onThreadShutdown(rawrbox::ENGINE_THREADS thread) override;
		void pollEvents() override;
		void update() override;
		void fixedUpdate() override;
		void draw() override;

	public:
		Game() = default;
		Game(const Game&) = delete;
		Game(Game&&) = delete;
		Game& operator=(const Game&) = delete;
		Game& operator=(Game&&) = delete;
		~Game() override = default;

		void drawWorld();

		void createBox(const rawrbox::Vector3f& pos, const rawrbox::Vector2f& size);

		void loadContent();
		void contentLoaded();
	};
} // namespace phys_2d_test
