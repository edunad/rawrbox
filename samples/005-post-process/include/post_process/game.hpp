#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/camera/orbital.hpp>
#include <rawrbox/render/model/assimp/model_imported.hpp>
#include <rawrbox/render/postprocess/manager.hpp>
#include <rawrbox/render/renderer.hpp>
#include <rawrbox/render/window.hpp>

#include <memory>

namespace post_process {
	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::Window> _window = nullptr;
		std::shared_ptr<rawrbox::Renderer> _render = nullptr;
		std::shared_ptr<rawrbox::CameraOrbital> _camera = nullptr;
		std::shared_ptr<rawrbox::PostProcessManager> _postProcess = nullptr;

		std::shared_ptr<rawrbox::ModelImported<>> _model = std::make_shared<rawrbox::ModelImported<>>();

		bool _rightClick = false;
		rawrbox::Vector2i _oldMousePos = {};

	public:
		using Engine::Engine;

		void init() override;
		void shutdown() override;
		void pollEvents() override;
		void update(float deltaTime, int64_t gameTime) override;
		void draw() override;

		void loadContent();
		void drawWorld();
	};
} // namespace post_process
