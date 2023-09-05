#include <rawrbox/assimp/importer.hpp>
#include <rawrbox/assimp/resources/model.hpp>
#include <rawrbox/render/camera/orbital.hpp>
#include <rawrbox/render/gizmos.hpp>
#include <rawrbox/render/materials/lit.hpp>
#include <rawrbox/render/materials/skinned.hpp>
#include <rawrbox/render/model/utils/mesh.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/keys.hpp>
#include <rawrbox/utils/threading.hpp>

#include <assimp/game.hpp>

#include <bx/bx.h>
#include <bx/math.h>

namespace assimp {

	void Game::setupGLFW() {
		this->_window = std::make_unique<rawrbox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("ASSIMP TEST");
		this->_window->setRenderer<>(
		    bgfx::RendererType::Count, []() {}, [this]() { this->drawWorld(); });
		this->_window->create(1024, 768, rawrbox::WindowFlags::Debug::TEXT | rawrbox::WindowFlags::Debug::PROFILER | rawrbox::WindowFlags::Window::WINDOWED | rawrbox::WindowFlags::Features::MULTI_THREADED);
		this->_window->onWindowClose += [this](auto& /*w*/) { this->shutdown(); };
	}

	void Game::init() {
		if (this->_window == nullptr) return;
		this->_window->initializeBGFX();

		// Setup camera
		auto cam = this->_window->setupCamera<rawrbox::CameraOrbital>(*this->_window);
		cam->setPos({0.F, 5.F, -5.F});
		cam->setAngle({0.F, bx::toRad(-45), 0.F, 0.F});
		// --------------

		rawrbox::RESOURCES::addLoader<rawrbox::FontLoader>();
		rawrbox::RESOURCES::addLoader<rawrbox::AssimpLoader>();

		// Load content ---
		this->loadContent();
		// -----
	}

	void Game::loadContent() {
		std::array initialContentFiles = {
		    std::make_pair<std::string, uint32_t>("cour.ttf", 0),
		    std::make_pair<std::string, uint32_t>("content/models/ps1_phasmophobia/Phasmaphobia_Semi.fbx", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_LIGHT),
		    std::make_pair<std::string, uint32_t>("content/models/wolf/wolfman_animated.fbx", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS),
		    std::make_pair<std::string, uint32_t>("content/models/vrm/MonoCat.glb", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS),
		    std::make_pair<std::string, uint32_t>("content/models/multiple_skeleton/twocubestest.gltf", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS | rawrbox::ModelLoadFlags::Debug::PRINT_BONE_STRUCTURE),
		    std::make_pair<std::string, uint32_t>("content/models/grandma_tv/scene.gltf", rawrbox::ModelLoadFlags::IMPORT_TEXTURES | rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS | rawrbox::ModelLoadFlags::Debug::PRINT_MATERIALS)};

		this->_loadingFiles = static_cast<int>(initialContentFiles.size());
		for (auto& f : initialContentFiles) {
			rawrbox::RESOURCES::loadFileAsync(f.first, f.second, [this]() {
				this->_loadingFiles--;
				if (this->_loadingFiles <= 0) {
					rawrbox::runOnRenderThread([this]() { this->contentLoaded(); });
				}
			});
		}

		this->_window->upload();
	}

	void Game::contentLoaded() {
		this->_font = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>("cour.ttf")->getSize(24);

		// Assimp test ---
		auto mdl = rawrbox::RESOURCES::getFile<rawrbox::ResourceAssimp>("./content/models/ps1_phasmophobia/Phasmaphobia_Semi.fbx")->get();

		this->_model->load(*mdl);
		this->_model->setPos({7, 1.1F, 0.F});
		this->_model->upload();

		this->_model2->load(*mdl);
		this->_model2->setPos({-6, 1.1F, 0.F});
		this->_model2->setMaterial<rawrbox::MaterialLit>();
		this->_model2->upload();

		// ANIMATIONS ---
		auto mdl2 = rawrbox::RESOURCES::getFile<rawrbox::ResourceAssimp>("./content/models/wolf/wolfman_animated.fbx")->get();
		this->_model3->setMaterial<rawrbox::MaterialSkinned>();
		this->_model3->load(*mdl2);
		this->_model3->playAnimation("Scene", true, 1.F);
		this->_model3->setPos({0, 0, 0});
		this->_model3->upload();

		auto mdl3 = rawrbox::RESOURCES::getFile<rawrbox::ResourceAssimp>("./content/models/multiple_skeleton/twocubestest.gltf")->get();
		this->_model4->setMaterial<rawrbox::MaterialSkinned>();
		this->_model4->load(*mdl3);
		this->_model4->playAnimation("MewAction", true, 0.8F);
		this->_model4->playAnimation("MewAction.001", true, 0.5F);
		this->_model4->setPos({0, 0, 2.5F});
		this->_model4->setScale({0.25F, 0.25F, 0.25F});
		this->_model4->upload();

		auto mdl4 = rawrbox::RESOURCES::getFile<rawrbox::ResourceAssimp>("./content/models/grandma_tv/scene.gltf")->get();
		this->_model5->setMaterial<rawrbox::MaterialSkinned>();
		this->_model5->load(*mdl4);
		this->_model5->playAnimation("Scene", true, 1.F);
		this->_model5->setPos({0, 0, -3.5F});
		this->_model5->setScale({0.35F, 0.35F, 0.35F});
		this->_model5->setEulerAngle({0, bx::toRad(90.F), 0});
		this->_model5->upload();

		auto mdl5 = rawrbox::RESOURCES::getFile<rawrbox::ResourceAssimp>("./content/models/vrm/MonoCat.glb")->get();
		this->_model6->setMaterial<rawrbox::MaterialSkinned>();
		this->_model6->load(*mdl5);
		this->_model6->setScale({1.2F, 1.2F, 1.2F});
		this->_model6->setPos({0.F, 0.F, -6.F});
		this->_model6->upload();

		//   -----

		// Text test ----
		{
			this->_text->addText(*this->_font, "TEXTURES + LIGHT", {-6.F, 3.0F, 0});
			this->_text->addText(*this->_font, "TEXTURES", {6.F, 3.0F, 0});
			this->_text->addText(*this->_font, "SINGLE ARMATURE +\nVERTEX ANIMATION", {0.F, 2.F, 0});
			this->_text->addText(*this->_font, "TWO ARMATURES +\nTWO ANIMATIONS", {0.F, 1.F, 2.5F});
			this->_text->addText(*this->_font, "VERTEX ANIMATIONS", {0.F, 1.8F, -3.5F});
			this->_text->addText(*this->_font, "EMBEDDED TEXTURES", {0.F, 1.8F, -6.F});
			this->_text->upload();
		}
		// ------

		{
			auto mesh = rawrbox::MeshUtils::generateGrid(24, {0.F, 0.F, 0.F});
			this->_modelGrid->addMesh(mesh);
			this->_modelGrid->upload();
		}

		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) return;

		this->_model.reset();
		this->_model2.reset();
		this->_model3.reset();
		this->_model4.reset();
		this->_model5.reset();
		this->_model6.reset();
		this->_modelGrid.reset();

		this->_text.reset();

		rawrbox::GIZMOS::shutdown();
		rawrbox::RESOURCES::shutdown();
		rawrbox::ASYNC::shutdown();

		this->_window->unblockPoll();
		this->_window.reset();
	}

	void Game::pollEvents() {
		if (this->_window == nullptr) return;
		this->_window->pollEvents();
	}

	void Game::update() {
		if (this->_window == nullptr) return;
		this->_window->update();
	}

	void Game::drawWorld() {
		if (!this->_ready) return;
		this->_modelGrid->draw();

		this->_model->draw();
		this->_model2->draw();
		this->_model3->draw();
		this->_model4->draw();
		this->_model5->draw();
		this->_model6->draw();

		this->_text->draw();
	}

	void Game::printFrames() {
		const bgfx::Stats* stats = bgfx::getStats();

		bgfx::dbgTextPrintf(1, 4, 0x6f, "GPU %0.6f [ms]", double(stats->gpuTimeEnd - stats->gpuTimeBegin) * 1000.0 / stats->gpuTimerFreq);
		bgfx::dbgTextPrintf(1, 5, 0x6f, "CPU %0.6f [ms]", double(stats->cpuTimeEnd - stats->cpuTimeBegin) * 1000.0 / stats->cpuTimerFreq);
		bgfx::dbgTextPrintf(1, 7, 0x5f, fmt::format("TRIANGLES: {}", stats->numPrims[bgfx::Topology::TriList]).c_str());
		bgfx::dbgTextPrintf(1, 8, 0x5f, fmt::format("DRAW CALLS: {}", stats->numDraw).c_str());
		bgfx::dbgTextPrintf(1, 9, 0x5f, fmt::format("COMPUTE CALLS: {}", stats->numCompute).c_str());
	}

	void Game::draw() {
		if (this->_window == nullptr) return;

		// DEBUG ----
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(1, 1, 0x1f, "003-assimp");
		bgfx::dbgTextPrintf(1, 2, 0x3f, "Description: ASSIMP model loading test");
		printFrames();
		// -----------

		if (!this->_ready) {
			bgfx::dbgTextPrintf(1, 10, 0x70, "                                   ");
			bgfx::dbgTextPrintf(1, 11, 0x70, "          LOADING CONTENT          ");
			bgfx::dbgTextPrintf(1, 12, 0x70, "                                   ");
		}

		// Draw DEBUG ---
		rawrbox::GIZMOS::draw();
		// -----------

		this->_window->render(); // Commit primitives
	}
} // namespace assimp
