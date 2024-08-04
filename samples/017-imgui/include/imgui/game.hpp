#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/models/model.hpp>
#include <rawrbox/render/textures/image.hpp>

namespace imgui {
	class Game : public rawrbox::Engine {
		bool _ready = false;

		// Engine setup ---
		void setupGLFW() override;
		void init() override;
		void onThreadShutdown(rawrbox::ENGINE_THREADS thread) override;
		void pollEvents() override;
		void update() override;
		void draw() override;
		// ----------------

		void drawIMGUIMenu();

		std::string _inputText = "blabalbal";
		rawrbox::TextureBase* _texture = nullptr;

	public:
		Game() = default;
		Game(const Game&) = delete;
		Game(Game&&) = delete;
		Game& operator=(const Game&) = delete;
		Game& operator=(Game&&) = delete;
		~Game() override = default;

		void drawOverlay();

		void loadContent();
		void contentLoaded();
	};
} // namespace imgui
