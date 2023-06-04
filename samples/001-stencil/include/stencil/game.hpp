#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/text/font.hpp>
#include <rawrbox/render/texture/gif.hpp>
#include <rawrbox/render/texture/image.hpp>
#include <rawrbox/render/window.hpp>

#include <memory>

namespace stencil {
	class Game : public rawrbox::Engine {
		std::shared_ptr<rawrbox::Window> _window = nullptr;
		std::shared_ptr<rawrbox::TextureImage> _texture = nullptr;
		std::shared_ptr<rawrbox::TextureGIF> _texture2 = nullptr;

		std::weak_ptr<rawrbox::Font> _font;
		std::weak_ptr<rawrbox::Font> _font2;
		std::weak_ptr<rawrbox::Font> _font3;

		bool _ready = false;
		std::atomic<int> _loadingFiles = 0;

		float _counter = 0;

		void setupGLFW() override;
		void init() override;

		void onThreadShutdown(rawrbox::ENGINE_THREADS thread) override;
		void pollEvents() override;
		void draw() override;

	public:
		void printFrames();

		void loadContent();
		void contentLoaded();

		void drawOverlay();
	};
} // namespace stencil
