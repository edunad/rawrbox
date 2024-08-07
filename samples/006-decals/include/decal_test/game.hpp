#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/materials/lit.hpp>
#include <rawrbox/render/models/model.hpp>
#include <rawrbox/render/textures/atlas.hpp>

#include <memory>

namespace decal_test {
	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::Model<>> _model = std::make_unique<rawrbox::Model<>>();
		std::unique_ptr<rawrbox::Model<rawrbox::MaterialLit>> _model2 = std::make_unique<rawrbox::Model<rawrbox::MaterialLit>>();

		bool _ready = false;

		void generateDecals();

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

		void drawWorld() const;
		void drawOverlay() const;

		void loadContent();
		void contentLoaded();
	};
} // namespace decal_test
