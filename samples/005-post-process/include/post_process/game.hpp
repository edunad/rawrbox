#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/models/model.hpp>
#include <rawrbox/render/post_process/manager.hpp>

#include <memory>

namespace post_process {
	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::PostProcessManager> _postProcess = nullptr;
		std::unique_ptr<rawrbox::Model<>> _model = std::make_unique<rawrbox::Model<>>();

		std::atomic<int> _loadingFiles = 0;
		bool _ready = false;

		void init() override;
		void setupGLFW() override;
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

		void loadContent();
		void contentLoaded();

		void drawWorld();
		void drawOverlay();
	};
} // namespace post_process
