#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/math/bbox.hpp>
#include <rawrbox/render/models/model.hpp>
#include <rawrbox/render/models/spline.hpp>
#include <rawrbox/render/models/text3D.hpp>

#include <memory>
#include <utility>
#include <vector>

namespace model {
	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::Model<>> _displacement = std::make_unique<rawrbox::Model<>>();
		std::unique_ptr<rawrbox::Model<>> _model = std::make_unique<rawrbox::Model<>>();
		std::unique_ptr<rawrbox::Model<>> _modelDynamic = std::make_unique<rawrbox::Model<>>();
		std::unique_ptr<rawrbox::Model<>> _sprite = std::make_unique<rawrbox::Model<>>();
		std::unique_ptr<rawrbox::Model<>> _sprite_2 = std::make_unique<rawrbox::Model<>>();
		std::unique_ptr<rawrbox::Spline<>> _spline = std::make_unique<rawrbox::Spline<>>();
		std::unique_ptr<rawrbox::Text3D<>> _text = std::make_unique<rawrbox::Text3D<>>();

		std::vector<std::pair<rawrbox::Vector3f, rawrbox::BBOXf>> _bboxes = {}; // Mesh pos + local bbox

		bool _ready = false;
		bool _debugDraw = true;

		// Engine setup ---
		void setupGLFW() override;
		void init() override;
		void onThreadShutdown(rawrbox::ENGINE_THREADS thread) override;
		void pollEvents() override;
		void update() override;
		void draw() override;
		// ----------------

		void createModels();
		void createSpline();
		void createDisplacement();
		void createSprite();
		void createText();
		void createDynamic();

		void loadContent();
		void contentLoaded();

	public:
		Game() = default;
		Game(const Game&) = delete;
		Game(Game&&) = delete;
		Game& operator=(const Game&) = delete;
		Game& operator=(Game&&) = delete;
		~Game() override = default;

		void drawWorld();
		void drawOverlay() const;
	};
} // namespace model
