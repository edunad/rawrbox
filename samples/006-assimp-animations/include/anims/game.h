#pragma once

#include <rawrbox/render/camera/perspective.h>
#include <rawrbox/render/engine.h>
#include <rawrbox/render/model/assimp/model_imported.h>
#include <rawrbox/render/postprocess/manager.h>
#include <rawrbox/render/renderer.h>
#include <rawrbox/render/window.h>

#include <memory>

namespace anims {
	class Game : public rawrBox::Engine {
		std::unique_ptr<rawrBox::Window> _window;
		std::shared_ptr<rawrBox::Renderer> _render;
		std::shared_ptr<rawrBox::CameraPerspective> _camera;
		std::shared_ptr<rawrBox::PostProcessManager> _postProcess;

		std::shared_ptr<rawrBox::ModelImported> _model;

		bool _rightClick = false;
		rawrBox::Vector2i _oldMousePos = {};

	public:
		using Engine::Engine;

		void init() override;
		void shutdown() override;
		void pollEvents() override;
		void update(float deltaTime, int64_t gameTime) override;
		void draw(const double alpha) override;

		void loadContent();
		void drawWorld();
	};
} // namespace anims
