#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/models/model.hpp>
#include <rawrbox/render/models/spline.hpp>
#include <rawrbox/render/models/sprite.hpp>
#include <rawrbox/render/models/text3D.hpp>

#include <memory>

namespace model {
	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::Model<>> _displacement = std::make_unique<rawrbox::Model<>>();
		std::unique_ptr<rawrbox::Model<>> _model = std::make_unique<rawrbox::Model<>>();
		std::unique_ptr<rawrbox::Model<>> _bboxes = std::make_unique<rawrbox::Model<>>();
		std::unique_ptr<rawrbox::Sprite<>> _sprite = std::make_unique<rawrbox::Sprite<>>();
		std::unique_ptr<rawrbox::Spline<>> _spline = std::make_unique<rawrbox::Spline<>>();
		std::unique_ptr<rawrbox::Text3D<>> _text = std::make_unique<rawrbox::Text3D<>>();

		bool _ready = false;

		void setupGLFW() override;
		void init() override;
		void onThreadShutdown(rawrbox::ENGINE_THREADS thread) override;
		void pollEvents() override;
		void update() override;
		void draw() override;

		void createModels();
		void createSpline();
		void createDisplacement();
		void createSprite();
		void createText();

		void loadContent();
		void contentLoaded();

		void drawWorld();

	public:
		Game() = default;
		Game(const Game&) = delete;
		Game(Game&&) = delete;
		Game& operator=(const Game&) = delete;
		Game& operator=(Game&&) = delete;
		~Game() override = default;
	};
} // namespace model
