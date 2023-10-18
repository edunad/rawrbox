#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/models/model.hpp>
#include <rawrbox/render/text/font.hpp>
#include <rawrbox/render/text/markdown.hpp>
#include <rawrbox/render/texture/animated.hpp>
#include <rawrbox/render/texture/image.hpp>
#include <rawrbox/render/window.hpp>

#include <memory>

namespace stencil {
	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::Window> _window = nullptr;
		std::unique_ptr<rawrbox::Model> _model = std::make_unique<rawrbox::Model>();
		std::unique_ptr<rawrbox::Markdown> _markdown = std::make_unique<rawrbox::Markdown>();

		rawrbox::TextureBase* _texture = nullptr;
		rawrbox::TextureBase* _texture2 = nullptr;
		rawrbox::TextureBase* _texture3 = nullptr;
		rawrbox::TextureBase* _texture4 = nullptr;
		rawrbox::TextureAnimatedBase* _texture5 = nullptr;

		rawrbox::Font* _font = nullptr;
		rawrbox::Font* _font2 = nullptr;
		rawrbox::Font* _font3 = nullptr;

		bool _ready = false;
		std::atomic<int> _loadingFiles = 0;

		float _counter = 0;

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

		void printFrames();

		void loadContent();
		void contentLoaded();

		void drawWorld();
		void drawOverlay();
	};
} // namespace stencil
