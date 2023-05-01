#pragma once

#include <rawrbox/render/camera/perspective.hpp>
#include <rawrbox/render/engine.hpp>
#include <rawrbox/render/model/assimp/model_imported.hpp>
#include <rawrbox/render/model/material/skinned_unlit.hpp>
#include <rawrbox/render/postprocess/manager.hpp>
#include <rawrbox/render/renderer.hpp>
#include <rawrbox/render/window.hpp>

#include <memory>

namespace anims {
	class Game : public rawrBox::Engine {
		std::unique_ptr<rawrBox::Window> _window = nullptr;
		std::shared_ptr<rawrBox::Renderer> _render = nullptr;
		std::shared_ptr<rawrBox::CameraPerspective> _camera = nullptr;
		std::shared_ptr<rawrBox::PostProcessManager> _postProcess = nullptr;

		std::shared_ptr<rawrBox::ModelImported<rawrBox::MaterialSkinnedUnlit>> _model = std::make_shared<rawrBox::ModelImported<rawrBox::MaterialSkinnedUnlit>>();
		std::shared_ptr<rawrBox::ModelImported<rawrBox::MaterialSkinnedUnlit>> _model2 = std::make_shared<rawrBox::ModelImported<rawrBox::MaterialSkinnedUnlit>>();
		std::shared_ptr<rawrBox::Model<>> _modelGrid = std::make_shared<rawrBox::Model<>>();

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