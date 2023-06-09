#pragma once

#include <rawrbox/bass/sound/instance.hpp>
#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/model/model.hpp>
#include <rawrbox/render/model/text3D.hpp>
#include <rawrbox/render/window.hpp>

#include <memory>

namespace bass_test {
	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::Window> _window = nullptr;

		std::unique_ptr<rawrbox::SoundInstance> _sound = nullptr;
		std::unique_ptr<rawrbox::SoundInstance> _sound2 = nullptr;

		std::unique_ptr<rawrbox::Model<>> _modelGrid = std::make_unique<rawrbox::Model<>>();

		std::unique_ptr<rawrbox::Text3D> _text = std::make_unique<rawrbox::Text3D>();
		std::unique_ptr<rawrbox::Text3D> _beatText = std::make_unique<rawrbox::Text3D>();

		rawrbox::Font* _font = nullptr;

		float _beat = 0;
		bool _ready = false;
		std::atomic<int> _loadingFiles = 0;

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

		void printFrames();

		void loadContent();
		void contentLoaded();
		void drawWorld();
	};
} // namespace bass_test
