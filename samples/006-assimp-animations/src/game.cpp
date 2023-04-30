
#include <rawrbox/render/model/material/skinned_unlit.hpp>
#include <rawrbox/render/model/mesh.hpp>
#include <rawrbox/utils/keys.hpp>
#include <rawrbox/utils/time.h>

#include <anims/game.h>
#include <bx/bx.h>
#include <bx/math.h>

#include <vector>

#include "rawrbox/render/model/assimp/model_imported.h"
#include "rawrbox/render/model/material/unlit.hpp"

namespace anims {
	void Game::init() {
		int width = 1024;
		int height = 768;

		this->_window = std::make_unique<rawrBox::Window>();
		this->_window->setMonitor(-1);
		this->_window->setTitle("ANIMATIONS TEST");
		this->_window->setRenderer(bgfx::RendererType::Count);
		this->_window->onResize += [this](auto& w, auto& size) {
			if (this->_render == nullptr) return;
			this->_render->resizeView(size);
		};

		this->_window->onWindowClose += [this](auto& w) {
			this->shutdown();
		};

		this->_window->onMouseKey += [this](auto& w, const rawrBox::Vector2i& mousePos, int button, int action, int mods) {
			const bool isDown = action == 1;
			if (button != MOUSE_BUTTON_2) return;

			this->_rightClick = isDown;
			this->_oldMousePos = mousePos;
		};

		this->_window->onMouseMove += [this](auto& w, const rawrBox::Vector2i& mousePos) {
			if (this->_camera == nullptr || !this->_rightClick) return;

			float m_mouseSpeed = 0.0015f;

			auto deltaX = mousePos.x - this->_oldMousePos.x;
			auto deltaY = mousePos.y - this->_oldMousePos.y;

			auto ang = this->_camera->getAngle();
			ang.x += m_mouseSpeed * static_cast<float>(deltaX);
			ang.y -= m_mouseSpeed * static_cast<float>(deltaY);

			this->_camera->setAngle(ang);
			this->_oldMousePos = mousePos;
		};

		this->_window->initialize(width, height, rawrBox::WindowFlags::Window::WINDOWED | rawrBox::WindowFlags::Debug::TEXT);

		this->_render = std::make_shared<rawrBox::Renderer>(0, this->_window->getSize());
		this->_render->setClearColor(0x00000000);
		// Setup camera
		this->_camera = std::make_shared<rawrBox::CameraPerspective>(this->_window->getAspectRatio(), 60.0f, 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
		this->_camera->setPos({0.f, 5.f, -5.f});
		this->_camera->setAngle({0.f, bx::toRad(-45), 0.f});
		// --------------

		// Load content ---
		this->loadContent();
		// -----
	}

	void Game::loadContent() {
		this->_render->upload();

		// Assimp test ---
		auto mat = std::make_shared<rawrBox::MaterialSkinnedUnlit>();
		auto meshMat = std::make_shared<rawrBox::MaterialUnlit>();

		this->_model = std::make_shared<rawrBox::ModelImported>(mat);
		this->_model->load("./content/models/wolf/wolfman_animated.fbx", rawrBox::ModelLoadFlags::IMPORT_TEXTURES | rawrBox::ModelLoadFlags::IMPORT_ANIMATIONS);
		this->_model->playAnimation("Scene", true, 1.5f);
		this->_model->setPos({0, 0, 0});
		this->_model->upload();

		this->_model2 = std::make_shared<rawrBox::ModelImported>(mat);
		this->_model2->load("./content/models/multiple_skeleton/twocubestest.gltf", rawrBox::ModelLoadFlags::IMPORT_TEXTURES | rawrBox::ModelLoadFlags::IMPORT_ANIMATIONS | rawrBox::ModelLoadFlags::Debug::PRINT_BONE_STRUCTURE);
		// this->_model2->playAnimation("MewAction", true, 0.8f);
		this->_model2->playAnimation("MewAction.001", true, 0.8f);
		this->_model2->setPos({0, 0, 0});
		this->_model2->upload();

		// -----

		this->_modelGrid = std::make_shared<rawrBox::Model>(meshMat);
		{
			auto mesh = rawrBox::ModelBase::generateCube(0, {0.05f, 0.05f, 0.05f});
			this->_modelGrid->addMesh(mesh);
		}
		{
			auto mesh = rawrBox::ModelBase::generateGrid(12, {0.f, 0.f, 0.f});
			this->_modelGrid->addMesh(mesh);
		}

		this->_modelGrid->upload();
		// -----
	}

	void Game::shutdown() {
		this->_render = nullptr;
		this->_model = nullptr;
		this->_modelGrid = nullptr;

		rawrBox::Engine::shutdown();
	}

	void Game::pollEvents() {
		if (this->_window == nullptr) return;
		this->_window->pollEvents();
	}

	void Game::update(float deltaTime, int64_t gameTime) {
		rawrBox::TimeUtils::deltaTime = deltaTime;
		if (this->_render == nullptr || this->_camera == nullptr) return;

		float m_moveSpeed = 5.f;

		auto dir = this->_camera->getForward();
		auto eye = this->_camera->getPos();
		auto right = this->_camera->getRight();

		auto m_dir = bx::Vec3(dir.x, dir.y, dir.z);
		auto m_eye = bx::Vec3(eye.x, eye.y, eye.z);

		if (this->_window->isKeyDown(KEY_LEFT_SHIFT)) {
			m_moveSpeed = 60.f;
		}

		if (this->_window->isKeyDown(KEY_W)) {
			m_eye = bx::mad(m_dir, deltaTime * m_moveSpeed, m_eye);
			this->_camera->setPos({m_eye.x, m_eye.y, m_eye.z});
		}

		if (this->_window->isKeyDown(KEY_S)) {
			m_eye = bx::mad(m_dir, -deltaTime * m_moveSpeed, m_eye);
			this->_camera->setPos({m_eye.x, m_eye.y, m_eye.z});
		}

		if (this->_window->isKeyDown(KEY_A)) {
			m_eye = bx::mad({right.x, right.y, right.z}, deltaTime * m_moveSpeed, m_eye);
			this->_camera->setPos({m_eye.x, m_eye.y, m_eye.z});
		}

		if (this->_window->isKeyDown(KEY_D)) {
			m_eye = bx::mad({right.x, right.y, right.z}, -deltaTime * m_moveSpeed, m_eye);
			this->_camera->setPos({m_eye.x, m_eye.y, m_eye.z});
		}
	}

	void Game::drawWorld() {
		// if (this->_model == nullptr || this->_modelGrid == nullptr || this->_model2 == nullptr) return;

		this->_modelGrid->draw(this->_camera->getPos());

		this->_model->draw(this->_camera->getPos());
		this->_model2->draw(this->_camera->getPos());
	}

	void Game::draw(const double alpha) {
		if (this->_render == nullptr) return;
		this->_render->swapBuffer(); // Clean up and set renderer

		bgfx::setViewTransform(rawrBox::CURRENT_VIEW_ID, this->_camera->getViewMtx().data(), this->_camera->getProjMtx().data());
		bgfx::dbgTextPrintf(1, 1, 0x0f, "ASSIMP ANIMATIONS TESTS ----------------------------------------------------------------------------------------");

		this->drawWorld();

		this->_render->render(); // Commit primitives
	}
} // namespace anims
