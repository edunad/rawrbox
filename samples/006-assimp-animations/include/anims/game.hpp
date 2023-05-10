#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/camera/orbital.hpp>
#include <rawrbox/render/model/assimp/model_imported.hpp>
#include <rawrbox/render/model/material/skinned_lit.hpp>
#include <rawrbox/render/model/material/skinned_unlit.hpp>
#include <rawrbox/render/model/text3D.hpp>
#include <rawrbox/render/postprocess/manager.hpp>
#include <rawrbox/render/renderer.hpp>
#include <rawrbox/render/text/engine.hpp>
#include <rawrbox/render/window.hpp>

#include <memory>

namespace anims {
	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::Window> _window = nullptr;
		std::shared_ptr<rawrbox::Renderer> _render = nullptr;
		std::shared_ptr<rawrbox::CameraOrbital> _camera = nullptr;
		std::shared_ptr<rawrbox::PostProcessManager> _postProcess = nullptr;
		std::unique_ptr<rawrbox::TextEngine> _textEngine = nullptr;

		std::shared_ptr<rawrbox::ModelImported<rawrbox::MaterialSkinnedUnlit>> _model = std::make_shared<rawrbox::ModelImported<rawrbox::MaterialSkinnedUnlit>>();
		std::shared_ptr<rawrbox::ModelImported<rawrbox::MaterialSkinnedUnlit>> _model2 = std::make_shared<rawrbox::ModelImported<rawrbox::MaterialSkinnedUnlit>>();
		std::shared_ptr<rawrbox::ModelImported<rawrbox::MaterialSkinnedLit>> _model3 = std::make_shared<rawrbox::ModelImported<rawrbox::MaterialSkinnedLit>>();
		std::shared_ptr<rawrbox::Model<>> _modelGrid = std::make_shared<rawrbox::Model<>>();
		std::shared_ptr<rawrbox::Text3D> _text = std::make_shared<rawrbox::Text3D>();

		rawrbox::Font* _font = nullptr;

	public:
		using Engine::Engine;

		void init() override;
		void shutdown() override;
		void pollEvents() override;
		void update() override;
		void draw() override;

		void loadContent();
		void drawWorld();
	};
} // namespace anims
