#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/models/model.hpp>
#include <rawrbox/render/post_process/base.hpp>

#include <memory>

namespace post_process {
	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::Model<>> _model = std::make_unique<rawrbox::Model<>>();
		bool _ready = false;

		rawrbox::PostProcessBase* _skybox = nullptr;
		rawrbox::PostProcessBase* _fog = nullptr;
		rawrbox::PostProcessBase* _dither = nullptr;
		rawrbox::PostProcessBase* _noise = nullptr;
		rawrbox::PostProcessBase* _bloom = nullptr;

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
		void drawOverlay() const;
	};
} // namespace post_process
