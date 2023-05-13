#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/text/engine.hpp>
#include <rawrbox/render/text/font.hpp>
#include <rawrbox/render/texture/gif.hpp>
#include <rawrbox/render/texture/image.hpp>
#include <rawrbox/render/window.hpp>

#include <memory>

namespace stencil {
	class Game : public rawrbox::Engine {
		std::shared_ptr<rawrbox::Window> _window = nullptr;
		std::unique_ptr<rawrbox::TextEngine> _textEngine = nullptr;

		std::shared_ptr<rawrbox::TextureImage> _texture = nullptr;
		std::shared_ptr<rawrbox::TextureGIF> _texture2 = nullptr;

		rawrbox::Font* _font = nullptr;
		rawrbox::Font* _font2 = nullptr;
		rawrbox::Font* _font3 = nullptr;

	public:
		float counter = 0;

		using Engine::Engine;

		void init() override;
		void shutdown() override;
		void pollEvents() override;
		void draw() override;

		void loadContent();
		void drawOverlay();
	};
} // namespace stencil
