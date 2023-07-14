#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/model/model.hpp>
#include <rawrbox/render/model/text3D.hpp>
#include <rawrbox/render/particles/system.hpp>
#include <rawrbox/render/window.hpp>

#include <memory>

namespace particle_test {
	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::Window> _window = nullptr;
		std::unique_ptr<rawrbox::Model<>> _modelGrid = std::make_unique<rawrbox::Model<>>();

		rawrbox::Emitter* _em = nullptr;
		std::unique_ptr<rawrbox::ParticleSystem<>> _ps = nullptr;

		std::unique_ptr<rawrbox::Text3D> _text = std::make_unique<rawrbox::Text3D>();
		rawrbox::Font* _font = nullptr;

		bool _ready = false;
		std::atomic<int> _loadingFiles = 0;

		void setupGLFW() override;
		void init() override;
		void onThreadShutdown(rawrbox::ENGINE_THREADS thread) override;
		void pollEvents() override;
		void update() override;
		void draw() override;

	public:
		Game() = default;
		Game(const Game&) = delete;
		Game(Game&&) = delete;
		Game& operator=(const Game&) = delete;
		Game& operator=(Game&&) = delete;
		~Game() override = default;

		void printFrames();

		void loadContent();
		void contentLoaded();
		void drawWorld();
	};
} // namespace particle_test
