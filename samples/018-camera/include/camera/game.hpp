#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/cameras/perspective.hpp>
#include <rawrbox/render/models/model.hpp>
#include <rawrbox/render/models/text3D.hpp>
#include <rawrbox/render/textures/image.hpp>

namespace camera {
	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::Model<>> _model = std::make_unique<rawrbox::Model<>>();
		std::unique_ptr<rawrbox::Model<>> _model2 = std::make_unique<rawrbox::Model<>>();
		std::unique_ptr<rawrbox::Model<>> _bboxes = std::make_unique<rawrbox::Model<>>();
		std::unique_ptr<rawrbox::Text3D<>> _text = std::make_unique<rawrbox::Text3D<>>();

		rawrbox::CameraPerspective* _camera = nullptr;

		bool _ready = false;
		bool _layers = true;

		// Engine setup ---
		void setupGLFW() override;
		void init() override;
		void onThreadShutdown(rawrbox::ENGINE_THREADS thread) override;
		void pollEvents() override;
		void update() override;
		void draw() override;
		// ----------------

	public:
		Game() = default;
		Game(const Game&) = delete;
		Game(Game&&) = delete;
		Game& operator=(const Game&) = delete;
		Game& operator=(Game&&) = delete;
		~Game() override = default;

		void drawWorld(uint32_t layer);
		void drawOverlay();

		void loadContent();
		void contentLoaded();
	};
} // namespace camera
