#pragma once

#include <rawrbox/render/engine.hpp>
#include <rawrbox/render/renderer.hpp>
#include <rawrbox/render/text/engine.hpp>
#include <rawrbox/render/text/font.hpp>
#include <rawrbox/render/texture/gif.hpp>
#include <rawrbox/render/texture/image.hpp>
#include <rawrbox/render/window.hpp>

#include <memory>

namespace stencil {
	class Game : public rawrBox::Engine {
		std::unique_ptr<rawrBox::Window> _window;
		std::shared_ptr<rawrBox::Renderer> _render;
		std::unique_ptr<rawrBox::TextEngine> _textEngine;

		std::shared_ptr<rawrBox::TextureImage> _texture;
		std::shared_ptr<rawrBox::TextureGIF> _texture2;

		rawrBox::Font* _font;
		rawrBox::Font* _font2;

	public:
		using Engine::Engine;

		void init() override;
		void shutdown() override;
		void pollEvents() override;
		void update(float deltaTime, int64_t gameTime) override;
		void draw(const double alpha) override;

		void loadContent();
		void drawOverlay();
	};
} // namespace stencil
