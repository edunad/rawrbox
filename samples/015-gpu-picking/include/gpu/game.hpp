#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/models/instanced.hpp>
#include <rawrbox/render/models/model.hpp>
#include <rawrbox/render/models/text3D.hpp>

#include <memory>

namespace gpu {
	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::Model<>> _model = std::make_unique<rawrbox::Model<>>();
		std::unique_ptr<rawrbox::InstancedModel<>> _instance = std::make_unique<rawrbox::InstancedModel<>>();

		std::unique_ptr<rawrbox::Text3D<>> _text = std::make_unique<rawrbox::Text3D<>>();

		bool _ready = false;
		rawrbox::Mesh<>* _lastPickedMesh = nullptr;
		rawrbox::Instance* _lastPickedInstance = nullptr;

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

		void drawWorld();

		void loadContent();
		void contentLoaded();
	};
} // namespace gpu
