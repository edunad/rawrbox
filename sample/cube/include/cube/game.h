#pragma once

#include <rawrbox/render/engine.h>
#include <rawrbox/render/renderer.h>
#include <rawrbox/render/texture.h>
#include <rawrbox/render/stencil.h>

#include <memory>

namespace cube {
	class Game : public rawrBox::Engine {
		bgfx::ViewId _id = 0;

		std::unique_ptr<rawrBox::Renderer> _render;
		std::unique_ptr<rawrBox::Stencil> _stencil;
		std::shared_ptr<rawrBox::Texture> _texture;

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
