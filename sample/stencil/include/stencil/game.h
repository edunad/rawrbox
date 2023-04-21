#pragma once

#include <rawrbox/render/engine.h>
#include <rawrbox/render/renderer.h>
#include <rawrbox/render/text/engine.h>
#include <rawrbox/render/text/font.h>
#include <rawrbox/render/texture/gif.h>
#include <rawrbox/render/texture/image.h>
#include <rawrbox/render/window.h>

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
		virtual void init() override;
		virtual void shutdown() override;
		virtual void pollEvents() override;
		virtual void update(float deltaTime, int64_t gameTime) override;
		virtual void draw(const double alpha) override;

		void loadContent();
		void drawOverlay();
	};
} // namespace stencil
