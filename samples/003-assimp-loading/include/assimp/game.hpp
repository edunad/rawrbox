#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/camera/orbital.hpp>
#include <rawrbox/render/model/assimp/assimp_model.hpp>
#include <rawrbox/render/model/material/lit.hpp>
#include <rawrbox/render/model/material/skinned_unlit.hpp>
#include <rawrbox/render/model/text3D.hpp>
#include <rawrbox/render/window.hpp>

#include <memory>

namespace assimp {
	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::Window> _window = nullptr;
		std::unique_ptr<rawrbox::CameraOrbital> _camera = nullptr;

		std::unique_ptr<rawrbox::AssimpModel<>> _model = std::make_unique<rawrbox::AssimpModel<>>();
		std::unique_ptr<rawrbox::AssimpModel<rawrbox::MaterialLit>> _model2 = std::make_unique<rawrbox::AssimpModel<rawrbox::MaterialLit>>();
		std::unique_ptr<rawrbox::AssimpModel<rawrbox::MaterialSkinnedUnlit>> _model3 = std::make_unique<rawrbox::AssimpModel<rawrbox::MaterialSkinnedUnlit>>();
		std::unique_ptr<rawrbox::AssimpModel<rawrbox::MaterialSkinnedUnlit>> _model4 = std::make_unique<rawrbox::AssimpModel<rawrbox::MaterialSkinnedUnlit>>();
		std::unique_ptr<rawrbox::AssimpModel<rawrbox::MaterialSkinnedUnlit>> _model5 = std::make_unique<rawrbox::AssimpModel<rawrbox::MaterialSkinnedUnlit>>();

		std::unique_ptr<rawrbox::Model<>> _modelGrid = std::make_unique<rawrbox::Model<>>();
		std::unique_ptr<rawrbox::Text3D> _text = std::make_unique<rawrbox::Text3D>();

		rawrbox::Font* _font = nullptr;

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
} // namespace assimp
