
#include <rawrbox/engine/static.hpp>
#include <rawrbox/render/camera/orbital.hpp>
#include <rawrbox/utils/keys.hpp>

#include <bx/math.h>

namespace rawrbox {
	CameraOrbital::CameraOrbital(rawrbox::Window* window, float speed) : CameraPerspective(window), _speed(speed) {
		window->onMouseKey += [this](auto& w, const rawrbox::Vector2i& mousePos, int button, int action, int mods) {
			const bool isDown = action == 1;
			if (button != MOUSE_BUTTON_2) return;

			this->_rightClick = isDown;
			this->_oldMousePos = mousePos;
		};

		window->onMouseMove += [this](auto& w, const rawrbox::Vector2i& mousePos) {
			if (!this->_rightClick) return;
			float m_mouseSpeed = 0.0015F;

			auto deltaX = mousePos.x - this->_oldMousePos.x;
			auto deltaY = mousePos.y - this->_oldMousePos.y;

			auto ang = this->getAngle();
			ang.x += m_mouseSpeed * static_cast<float>(deltaX);
			ang.y -= m_mouseSpeed * static_cast<float>(deltaY);

			this->setAngle(ang);
			this->_oldMousePos = mousePos;
		};
	}

	void CameraOrbital::update() {
		auto dir = this->getForward();
		auto eye = this->getPos();
		auto right = this->getRight();

		auto m_dir = bx::Vec3(dir.x, dir.y, dir.z);
		auto m_eye = bx::Vec3(eye.x, eye.y, eye.z);

		float sp = this->_speed;
		if (this->_window->isKeyDown(KEY_LEFT_SHIFT)) {
			sp *= 2.F;
		}

		if (this->_window->isKeyDown(KEY_W)) {
			m_eye = bx::mad(m_dir, rawrbox::DELTA_TIME * sp, m_eye);
			this->setPos({m_eye.x, m_eye.y, m_eye.z});
		}

		if (this->_window->isKeyDown(KEY_S)) {
			m_eye = bx::mad(m_dir, -rawrbox::DELTA_TIME * sp, m_eye);
			this->setPos({m_eye.x, m_eye.y, m_eye.z});
		}

		if (this->_window->isKeyDown(KEY_A)) {
			m_eye = bx::mad({right.x, right.y, right.z}, rawrbox::DELTA_TIME * sp, m_eye);
			this->setPos({m_eye.x, m_eye.y, m_eye.z});
		}

		if (this->_window->isKeyDown(KEY_D)) {
			m_eye = bx::mad({right.x, right.y, right.z}, -rawrbox::DELTA_TIME * sp, m_eye);
			this->setPos({m_eye.x, m_eye.y, m_eye.z});
		}
	}
} // namespace rawrbox
