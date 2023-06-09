#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/camera/orbital.hpp>
#include <rawrbox/render/model/assimp/assimp_model.hpp>
#include <rawrbox/render/postprocess/manager.hpp>
#include <rawrbox/render/window.hpp>

#include <memory>

namespace post_process {
	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::Window> _window = nullptr;
		std::unique_ptr<rawrbox::CameraOrbital> _camera = nullptr;
		std::unique_ptr<rawrbox::PostProcessManager> _postProcess = nullptr;

		std::unique_ptr<rawrbox::AssimpModel<>> _model = std::make_unique<rawrbox::AssimpModel<>>();

		std::atomic<int> _loadingFiles = 0;
		bool _ready = false;

		void init() override;
		void setupGLFW() override;
		void onThreadShutdown(rawrbox::ENGINE_THREADS thread) override;
		void pollEvents() override;
		void update() override;
		void draw() override;

	public:
		void printFrames();

		void loadContent();
		void contentLoaded();
		void drawWorld();
	};
} // namespace post_process
