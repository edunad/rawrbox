#pragma once

#include <rawrbox/assimp/model.hpp>
#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/materials/lit.hpp>
#include <rawrbox/render/materials/skinned.hpp>
#include <rawrbox/render/materials/skinnedLit.hpp>
#include <rawrbox/render/models/text3D.hpp>

#include <memory>

namespace assimp {
	class Game : public rawrbox::Engine {
		std::unique_ptr<rawrbox::AssimpModel<>> _model = std::make_unique<rawrbox::AssimpModel<>>();
		std::unique_ptr<rawrbox::AssimpModel<rawrbox::MaterialLit>> _model2 = std::make_unique<rawrbox::AssimpModel<rawrbox::MaterialLit>>();

		std::unique_ptr<rawrbox::AssimpModel<rawrbox::MaterialSkinned>> _model3 = std::make_unique<rawrbox::AssimpModel<rawrbox::MaterialSkinned>>();
		std::unique_ptr<rawrbox::AssimpModel<rawrbox::MaterialSkinnedLit>> _model4 = std::make_unique<rawrbox::AssimpModel<rawrbox::MaterialSkinnedLit>>();

		std::unique_ptr<rawrbox::AssimpModel<rawrbox::MaterialSkinned>> _model5 = std::make_unique<rawrbox::AssimpModel<rawrbox::MaterialSkinned>>();
		std::unique_ptr<rawrbox::AssimpModel<rawrbox::MaterialSkinned>> _model6 = std::make_unique<rawrbox::AssimpModel<rawrbox::MaterialSkinned>>();

		std::unique_ptr<rawrbox::AssimpModel<>> _model7 = std::make_unique<rawrbox::AssimpModel<>>();
		std::unique_ptr<rawrbox::AssimpModel<rawrbox::MaterialSkinned>> _model8 = std::make_unique<rawrbox::AssimpModel<rawrbox::MaterialSkinned>>();

		std::unique_ptr<rawrbox::Model<>> _modelGrid = std::make_unique<rawrbox::Model<>>();
		std::unique_ptr<rawrbox::Text3D<>> _text = std::make_unique<rawrbox::Text3D<>>();

		bool _ready = false;

		void init() override;
		void setupGLFW() override;
		void onThreadShutdown(rawrbox::ENGINE_THREADS thread) override;
		void pollEvents() override;
		void update() override;
		void draw() override;

		void testANIM();

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
} // namespace assimp
