#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/materials/lit.hpp>
#include <rawrbox/render/model/assimp/assimp_model.hpp>
#include <rawrbox/render/model/model.hpp>
#include <rawrbox/render/texture/atlas.hpp>
#include <rawrbox/render/window.hpp>

#include <memory>

namespace decal_test {
	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::Window> _window = nullptr;
		std::unique_ptr<rawrbox::Model<>> _model = std::make_unique<rawrbox::Model<>>();
		std::unique_ptr<rawrbox::AssimpModel<>> _model2 = std::make_unique<rawrbox::AssimpModel<>>();
		std::unique_ptr<rawrbox::AssimpModel<rawrbox::MaterialLit>> _model3 = std::make_unique<rawrbox::AssimpModel<rawrbox::MaterialLit>>();

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

		void drawWorld();
		void printFrames();

		void loadContent();
		void contentLoaded();
	};
} // namespace decal_test