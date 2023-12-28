#pragma once

#include <rawrbox/bass/sound/instance.hpp>
#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/models/text3D.hpp>

namespace bass_test {
	class Game : public rawrbox::Engine {

		std::weak_ptr<rawrbox::SoundInstance> _sound;
		std::weak_ptr<rawrbox::SoundInstance> _sound2;

		std::unique_ptr<rawrbox::Model<>> _modelGrid = std::make_unique<rawrbox::Model<>>();
		std::unique_ptr<rawrbox::Text3D<>> _text = std::make_unique<rawrbox::Text3D<>>();
		std::unique_ptr<rawrbox::Text3D<>> _beatText = std::make_unique<rawrbox::Text3D<>>();

		rawrbox::Font* _font = nullptr;

		std::atomic<int> _loadingFiles = 0;
		bool _ready = false;
		float _beat = 0.0F;

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
	};
} // namespace bass_test
