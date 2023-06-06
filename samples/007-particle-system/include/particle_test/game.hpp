#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/camera/orbital.hpp>
#include <rawrbox/render/model/model.hpp>
#include <rawrbox/render/model/text3D.hpp>
#include <rawrbox/render/particles/system.hpp>
#include <rawrbox/render/window.hpp>

#include <memory>

namespace particle_test {
	class Game : public rawrbox::Engine {
		std::shared_ptr<rawrbox::Window> _window = nullptr;
		std::shared_ptr<rawrbox::CameraOrbital> _camera = nullptr;

		std::shared_ptr<rawrbox::Model<>> _modelGrid = std::make_shared<rawrbox::Model<>>();

		std::shared_ptr<rawrbox::Emitter> _em = nullptr;
		std::shared_ptr<rawrbox::ParticleSystem<>> _ps = nullptr;

		std::shared_ptr<rawrbox::Text3D> _text = std::make_shared<rawrbox::Text3D>();
		rawrbox::Font* _font;

		bool _ready = false;
		std::atomic<int> _loadingFiles = 0;

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
} // namespace particle_test
