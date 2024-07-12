#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/models/model.hpp>
#include <rawrbox/render/textures/image.hpp>

namespace steamworks {
	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::TextureImage> _avatar = nullptr;
		std::vector<std::unique_ptr<rawrbox::TextureImage>> _friendAvatars = {};

		std::unique_ptr<rawrbox::Model<>> _model = std::make_unique<rawrbox::Model<>>();

		void setupGLFW() override;
		void init() override;
		void onThreadShutdown(rawrbox::ENGINE_THREADS thread) override;
		void pollEvents() override;
		void update() override;

		void draw() override;

		bool _ready = false;
		float _counter = 0.F;

	public:
		Game() = default;
		Game(const Game&) = delete;
		Game(Game&&) = delete;
		Game& operator=(const Game&) = delete;
		Game& operator=(Game&&) = delete;
		~Game() override = default;

		void drawWorld();
		void drawOverlay() const;

		void loadContent();
		void contentLoaded();
	};
} // namespace steamworks
