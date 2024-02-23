
#include <rawrbox/engine/static.hpp>
#include <rawrbox/render/cameras/orbital.hpp>

namespace rawrbox {
	CameraOrbital::CameraOrbital(rawrbox::Window& window, float speed, float FOV, float near, float far) : CameraPerspective(window.getSize(), FOV, near, far), _window(&window), _speed(speed) {
		this->_window->onMouseKey += [this](auto& /*w*/, const rawrbox::Vector2i& mousePos, int button, int action, int /*mods*/) {
			const bool isDown = action == 1;
			if (button != this->_controls.rotate) return;

			this->_enableLook = isDown;
			this->_oldMousePos = mousePos;

			if (isDown && this->onMovementStart) this->onMovementStart();
			if (!isDown && this->onMovementStop) this->onMovementStop();
		};

		this->_window->onMouseMove += [this](auto& /*w*/, const rawrbox::Vector2i& mousePos) {
			if (!this->_enableLook) return;
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

	void CameraOrbital::setControls(rawrbox::CameraControls controls) { this->_controls = controls; }

	void CameraOrbital::update() {
		if (this->_window == nullptr) return;

		auto dir = this->getForward();
		auto eye = this->getPos();
		auto right = this->getRight();

		auto m_dir = rawrbox::Vector3f(dir.x, dir.y, dir.z);
		auto m_eye = rawrbox::Vector3f(eye.x, eye.y, eye.z);

		float sp = this->_speed;
		if (this->_window->isKeyDown(KEY_LEFT_SHIFT)) {
			sp *= 2.F;
		}

		if (this->_window->isKeyDown(this->_controls.forward)) {
			m_eye = rawrbox::Vector3f::mad(m_dir, rawrbox::DELTA_TIME * sp, m_eye);
			this->setPos({m_eye.x, m_eye.y, m_eye.z});
		}

		if (this->_window->isKeyDown(this->_controls.backwards)) {
			m_eye = rawrbox::Vector3f::mad(m_dir, -rawrbox::DELTA_TIME * sp, m_eye);
			this->setPos({m_eye.x, m_eye.y, m_eye.z});
		}

		if (this->_window->isKeyDown(this->_controls.left)) {
			m_eye = rawrbox::Vector3f::mad({right.x, right.y, right.z}, rawrbox::DELTA_TIME * sp, m_eye);
			this->setPos({m_eye.x, m_eye.y, m_eye.z});
		}

		if (this->_window->isKeyDown(this->_controls.right)) {
			m_eye = rawrbox::Vector3f::mad({right.x, right.y, right.z}, -rawrbox::DELTA_TIME * sp, m_eye);
			this->setPos({m_eye.x, m_eye.y, m_eye.z});
		}
	}
} // namespace rawrbox
