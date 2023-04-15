#pragma once

#include <rawrbox/render/engine.h>
#include <rawrbox/render/renderer.h>
#include <rawrbox/render/window.h>

#include <rawrbox/render/texture/gif.h>
#include <rawrbox/render/texture/image.h>

#include <rawrbox/render/text/engine.h>
#include <rawrbox/render/text/font.h>

#include <rawrbox/render/camera/perspective.h>

#include <rawrbox/render/model/model.h>

#include <rawrbox/render/stencil.h>

#include <memory>

namespace cube {
	class Game : public rawrBox::Engine {
		std::unique_ptr<rawrBox::Window> _window;
		std::shared_ptr<rawrBox::Renderer> _render;
		std::shared_ptr<rawrBox::CameraPerspective> _camera;
		std::unique_ptr<rawrBox::TextEngine> _textEngine;

		std::shared_ptr<rawrBox::TextureImage> _texture;
		std::shared_ptr<rawrBox::TextureGIF> _texture2;

		std::shared_ptr<rawrBox::Model> _model;
		std::shared_ptr<rawrBox::Model> _model2;

		rawrBox::Font* _font;
		rawrBox::Font* _font2;

		bool _rightClick = false;
		rawrBox::Vector2i _oldMousePos = {};

	public:
		virtual void init() override;
		virtual void shutdown() override;
		virtual void pollEvents() override;
		virtual void update(float deltaTime, int64_t gameTime) override;
		virtual void draw(const double alpha) override;
	};
} // namespace cube
