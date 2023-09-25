#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/model/instanced.hpp>
#include <rawrbox/render/model/model.hpp>
#include <rawrbox/render/model/text3D.hpp>
#include <rawrbox/render/text/engine.hpp>
#include <rawrbox/render/window.hpp>

#include <memory>

namespace gpu {
	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::Window> _window = nullptr;
		std::unique_ptr<rawrbox::Model> _model = std::make_unique<rawrbox::Model>();
		std::unique_ptr<rawrbox::InstancedModel> _instance = std::make_unique<rawrbox::InstancedModel>();
		std::unique_ptr<rawrbox::Text3D> _text = std::make_unique<rawrbox::Text3D>();

		rawrbox::Font* _font = nullptr;

		rawrbox::VertexData* _lastPicked_vert = nullptr;
		rawrbox::Mesh* _lastPicked_mesh = nullptr;
		rawrbox::Instance* _lastPicked_instance = nullptr;

		std::atomic<int> _loadingFiles = 0;
		bool _ready = false;

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
		void drawOverlay();
	};
} // namespace gpu
