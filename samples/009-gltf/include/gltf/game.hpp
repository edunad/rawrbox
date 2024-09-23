#pragma once

//-------------- TEMP
#include <rawrbox/gltf/importer.hpp>
///----------

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/gltf/model.hpp>
#include <rawrbox/render/materials/skinned.hpp>

namespace gltf {
	class Game : public rawrbox::Engine {
		bool _ready = false;
		float _time = 0.F;

		// TEMP ---
		std::unique_ptr<rawrbox::GLTFImporter> _tst = nullptr;
		std::unique_ptr<rawrbox::GLTFModel<rawrbox::MaterialSkinned>> _tstMdl = nullptr;
		// ------

		void init() override;
		void setupGLFW() override;
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

		void loadContent();
		void contentLoaded();

		void drawWorld();
	};
} // namespace gltf
