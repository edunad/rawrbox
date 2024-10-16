#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/gltf/model.hpp>
#include <rawrbox/render/materials/lit.hpp>
#include <rawrbox/render/materials/skinned.hpp>
#include <rawrbox/render/materials/skinnedLit.hpp>
#include <rawrbox/render/models/text3D.hpp>

namespace gltf {
	class Game : public rawrbox::Engine {

		std::unique_ptr<rawrbox::GLTFModel<>> _blendTest;
		std::unique_ptr<rawrbox::GLTFModel<>> _grandmaTV;

		std::unique_ptr<rawrbox::GLTFModel<rawrbox::MaterialLit>> _phasmo;

		std::unique_ptr<rawrbox::GLTFModel<rawrbox::MaterialSkinned>> _animTest;

		std::unique_ptr<rawrbox::GLTFModel<rawrbox::MaterialSkinned>> _wolf;
		std::unique_ptr<rawrbox::GLTFModel<rawrbox::MaterialSkinnedLit>> _wolfLit;

		std::unique_ptr<rawrbox::Model<>> _modelGrid;
		std::unique_ptr<rawrbox::Model<>> _bbox;

		std::unique_ptr<rawrbox::Text3D<>> _text;

		bool _ready = false;

		void playTestAnim();

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
