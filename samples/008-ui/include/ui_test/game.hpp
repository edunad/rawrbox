#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/window.hpp>
#include <rawrbox/ui/elements/anim.hpp>
#include <rawrbox/ui/elements/graph.hpp>
#include <rawrbox/ui/elements/group.hpp>
#include <rawrbox/ui/elements/image.hpp>
#include <rawrbox/ui/root.hpp>

#include <memory>

namespace ui_test {
	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::Window> _window = nullptr;
		std::unique_ptr<rawrbox::UIRoot> _ROOT_UI = nullptr;

		std::unique_ptr<rawrbox::UIAnim<rawrbox::UIImage>> _anim = nullptr;
		rawrbox::UIGraph* _graph = nullptr;
		rawrbox::UIGroup* _group = nullptr;

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
	};
} // namespace ui_test
