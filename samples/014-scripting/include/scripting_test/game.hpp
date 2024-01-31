#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/models/instanced.hpp>
#include <rawrbox/utils/console.hpp>

#ifdef RAWRBOX_UI
	#include <rawrbox/ui/root.hpp>
#endif

namespace scripting_test {
	class Game : public rawrbox::Engine {
#ifdef RAWRBOX_UI
		std::unique_ptr<rawrbox::UIRoot> _ROOT_UI = nullptr;
#endif

		std::shared_ptr<rawrbox::Model<>> _model = std::make_shared<rawrbox::Model<>>();
		std::shared_ptr<rawrbox::InstancedModel<>> _instance = std::make_shared<rawrbox::InstancedModel<>>();
		std::unique_ptr<rawrbox::Console> _console = std::make_unique<rawrbox::Console>();

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

		void drawWorld();
		void drawOverlay();

		void loadContent();
		void contentLoaded();
	};
} // namespace scripting_test
