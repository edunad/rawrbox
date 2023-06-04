#pragma once

#include <rawrbox/bass/sound/instance.hpp>
#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/camera/orbital.hpp>
#include <rawrbox/render/model/model.hpp>
#include <rawrbox/render/model/text3D.hpp>
#include <rawrbox/render/window.hpp>

#include <memory>

namespace bass_test {
	class Game : public rawrbox::Engine {
		std::shared_ptr<rawrbox::Window> _window = nullptr;
		std::shared_ptr<rawrbox::CameraOrbital> _camera = nullptr;

		std::shared_ptr<rawrbox::SoundInstance> _sound = nullptr;
		std::shared_ptr<rawrbox::SoundInstance> _sound2 = nullptr;

		std::shared_ptr<rawrbox::Model<>> _modelGrid = std::make_shared<rawrbox::Model<>>();

		std::shared_ptr<rawrbox::Text3D> _text = std::make_shared<rawrbox::Text3D>();
		std::shared_ptr<rawrbox::Text3D> _beatText = std::make_shared<rawrbox::Text3D>();

		std::weak_ptr<rawrbox::Font> _font;

		float _beat = 0;
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
} // namespace bass_test
