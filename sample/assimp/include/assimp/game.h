#pragma once

#include <rawrbox/render/camera/perspective.h>
#include <rawrbox/render/engine.h>
#include <rawrbox/render/model/assimp/model_imported.h>
#include <rawrbox/render/renderer.h>
#include <rawrbox/render/window.h>

#include <memory>

namespace assimp {
	class Game : public rawrBox::Engine {
		std::unique_ptr<rawrBox::Window> _window;
		std::shared_ptr<rawrBox::Renderer> _render;
		std::shared_ptr<rawrBox::CameraPerspective> _camera;

		std::shared_ptr<rawrBox::ModelImported> _model;

		bool _rightClick = false;
		rawrBox::Vector2i _oldMousePos = {};

	public:
		virtual void init() override;
		virtual void shutdown() override;
		virtual void pollEvents() override;
		virtual void update(float deltaTime, int64_t gameTime) override;
		virtual void draw(const double alpha) override;

		void loadContent();
		void drawWorld();
	};
} // namespace assimp
