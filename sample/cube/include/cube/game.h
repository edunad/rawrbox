#pragma once

#include <rawrbox/render/engine.h>
#include <rawrbox/render/window.h>
#include <rawrbox/render/renderer.h>
#include <rawrbox/render/texture.h>
#include <rawrbox/render/animated_texture.h>
#include <rawrbox/render/stencil.h>

#include <memory>

namespace cube {
	class Game : public rawrBox::Engine {
		std::shared_ptr<rawrBox::Texture> _texture;
		std::shared_ptr<rawrBox::Texture> _texture2;
		std::shared_ptr<rawrBox::AnimatedTexture> _texture3;
		std::shared_ptr<rawrBox::AnimatedTexture> _texture4;

		std::unique_ptr<rawrBox::Renderer> _render;
		std::unique_ptr<rawrBox::Window> _window;

		float _view[16];
		float _proj[16];

	public:

		virtual void init() override;
		virtual void shutdown() override;
		virtual void pollEvents() override;
		virtual void update(float deltaTime, int64_t gameTime) override;
		virtual void draw(const double alpha) override;
	};
}
