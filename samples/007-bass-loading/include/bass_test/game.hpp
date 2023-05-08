#pragma once

#include <rawrbox/bass/sound/instance.hpp>
#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/camera/orbital.hpp>
#include <rawrbox/render/model/model.hpp>
#include <rawrbox/render/model/text3D.hpp>
#include <rawrbox/render/renderer.hpp>
#include <rawrbox/render/text/engine.hpp>
#include <rawrbox/render/window.hpp>

#include <memory>

namespace bass_test {
	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::Window> _window = nullptr;
		std::shared_ptr<rawrbox::Renderer> _render = nullptr;
		std::shared_ptr<rawrbox::CameraOrbital> _camera = nullptr;

		std::shared_ptr<rawrbox::SoundInstance> _sound = nullptr;
		std::shared_ptr<rawrbox::SoundInstance> _sound2 = nullptr;

		std::shared_ptr<rawrbox::Model<>> _modelGrid = std::make_shared<rawrbox::Model<>>();

		std::unique_ptr<rawrbox::TextEngine> _textEngine = nullptr;

		std::shared_ptr<rawrbox::Text3D> _text = std::make_shared<rawrbox::Text3D>();
		std::shared_ptr<rawrbox::Text3D> _beatText = std::make_shared<rawrbox::Text3D>();

		rawrbox::Font* _font = nullptr;

		float _beat = 0;

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
} // namespace bass_test
