#pragma once

#include <rawrbox/bass/sound/instance.hpp>
#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/camera/perspective.hpp>
#include <rawrbox/render/model/model.hpp>
#include <rawrbox/render/model/text3D.hpp>
#include <rawrbox/render/renderer.hpp>
#include <rawrbox/render/text/engine.hpp>
#include <rawrbox/render/window.hpp>

#include <memory>

namespace bass_test {
	class Game : public rawrBox::Engine {
		std::unique_ptr<rawrBox::Window> _window = nullptr;
		std::shared_ptr<rawrBox::Renderer> _render = nullptr;
		std::shared_ptr<rawrBox::CameraPerspective> _camera = nullptr;

		std::shared_ptr<rawrBox::SoundInstance> _sound = nullptr;
		std::shared_ptr<rawrBox::Model<>> _modelGrid = std::make_shared<rawrBox::Model<>>();
		std::unique_ptr<rawrBox::TextEngine> _textEngine = nullptr;
		std::shared_ptr<rawrBox::Text3D> _text = std::make_shared<rawrBox::Text3D>();

		rawrBox::Font* _font = nullptr;

		float _beat = 0;
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
} // namespace bass_test
