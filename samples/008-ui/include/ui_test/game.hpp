#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/window.hpp>

#include <memory>

namespace ui_test {
	class Game : public rawrbox::Engine {
		std::shared_ptr<rawrbox::Window> _window = nullptr;

	public:
		using Engine::Engine;

		void init() override;
		void shutdown() override;
		void pollEvents() override;
		void update() override;
		void draw() override;

		void loadContent();
		void contentLoaded();
	};
} // namespace ui_test
