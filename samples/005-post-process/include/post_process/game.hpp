#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/camera/orbital.hpp>
#include <rawrbox/render/model/assimp/assimp_model.hpp>
#include <rawrbox/render/postprocess/manager.hpp>
#include <rawrbox/render/window.hpp>

#include <memory>

namespace post_process {
	class Game : public rawrbox::Engine {
		std::shared_ptr<rawrbox::Window> _window = nullptr;
		std::shared_ptr<rawrbox::CameraOrbital> _camera = nullptr;
		std::shared_ptr<rawrbox::PostProcessManager> _postProcess = nullptr;

		std::shared_ptr<rawrbox::AssimpModel<>> _model = std::make_shared<rawrbox::AssimpModel<>>();

		bool _ready = false;

		void init() override;
		void setupGLFW() override;

	public:
		using Engine::Engine;

		void shutdown() override;
		void pollEvents() override;
		void update() override;
		void draw() override;

		void loadContent();
		void contentLoaded();
		void drawWorld();
	};
} // namespace post_process
