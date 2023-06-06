#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/camera/orbital.hpp>
#include <rawrbox/render/model/assimp/assimp_model.hpp>
#include <rawrbox/render/model/material/lit.hpp>
#include <rawrbox/render/model/text3D.hpp>
#include <rawrbox/render/text/engine.hpp>
#include <rawrbox/render/window.hpp>

#include <memory>

namespace light {
	class Game : public rawrbox::Engine {
		std::shared_ptr<rawrbox::Window> _window = nullptr;
		std::shared_ptr<rawrbox::CameraOrbital> _camera = nullptr;
		std::unique_ptr<rawrbox::TextEngine> _textEngine = nullptr;

		std::shared_ptr<rawrbox::AssimpModel<rawrbox::MaterialLit>> _model = std::make_shared<rawrbox::AssimpModel<rawrbox::MaterialLit>>();
		std::shared_ptr<rawrbox::Text3D> _text = std::make_shared<rawrbox::Text3D>();

		rawrbox::Font* _font;

		std::atomic<int> _loadingFiles = 0;
		bool _ready = false;

		void setupGLFW() override;
		void init() override;
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
} // namespace light
