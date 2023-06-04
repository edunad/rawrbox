#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/window.hpp>
#include <rawrbox/ui/elements/anim.hpp>
#include <rawrbox/ui/elements/graph.hpp>
#include <rawrbox/ui/elements/image.hpp>
#include <rawrbox/ui/root.hpp>

#include <memory>

namespace ui_test {
	class Game : public rawrbox::Engine {
		std::shared_ptr<rawrbox::Window> _window = nullptr;
		std::shared_ptr<rawrbox::UIRoot> _ROOT_UI = nullptr;

		std::shared_ptr<rawrbox::UIAnim<rawrbox::UIImage>> _anim = nullptr;
		std::shared_ptr<rawrbox::UIGraph> _graph = nullptr;

		bool _ready = false;

		void setupGLFW() override;
		void init() override;

	public:
		using Engine::Engine;

		void shutdown() override;
		void pollEvents() override;
		void update() override;
		void draw() override;

		void loadContent();
		void contentLoaded();
	};
} // namespace ui_test
