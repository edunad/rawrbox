#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/model/model.hpp>
#include <rawrbox/render/model/spline.hpp>
#include <rawrbox/render/model/sprite.hpp>
#include <rawrbox/render/model/text3D.hpp>
#include <rawrbox/render/window.hpp>

#include <memory>

namespace model {
	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::Window> _window = nullptr;

		std::unique_ptr<rawrbox::Model<>> _displacement = std::make_unique<rawrbox::Model<>>();
		std::unique_ptr<rawrbox::Model<>> _model = std::make_unique<rawrbox::Model<>>();
		std::unique_ptr<rawrbox::Model<>> _bboxes = std::make_unique<rawrbox::Model<>>();
		std::unique_ptr<rawrbox::Sprite<>> _sprite = std::make_unique<rawrbox::Sprite<>>();
		std::unique_ptr<rawrbox::Spline<>> _spline = std::make_unique<rawrbox::Spline<>>();
		std::unique_ptr<rawrbox::Text3D> _text = std::make_unique<rawrbox::Text3D>();

		rawrbox::Font* _font = nullptr;

		std::atomic<int> _loadingFiles = 0;
		bool _ready = false;

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

		void printFrames();

		void loadContent();
		void contentLoaded();

		void drawWorld();
	};
} // namespace model
