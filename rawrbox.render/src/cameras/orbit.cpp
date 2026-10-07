
#include <rawrbox/engine/static.hpp>
#include <rawrbox/math/pi.hpp>
#include <rawrbox/render/cameras/orbit.hpp>

#include <algorithm>

namespace rawrbox {
	constexpr float PITCH_LIMIT = rawrbox::pi<float> / 2.F - 0.01F;

	CameraOrbit::CameraOrbit(rawrbox::Window& window, float FOV, float near, float far) : rawrbox::CameraPerspective(window.getSize(), FOV, near, far), _window(&window) {
		this->_window->onMouseKey += [this](auto& /*w*/, const rawrbox::Vector2i& mousePos, int button, int action, int /*mods*/) {
			if (!this->_controlsEnabled || !this->isEnabled()) {
				this->_looking = false;
				this->_panning = false;
				return;
			}

			const bool isDown = action == 1;
			if (button != this->_controls.pan && button != this->_controls.look) return;

			if (isDown) {
				if (this->_dragButton != -1) return;
				if (this->_mouseCheck != nullptr && !this->_mouseCheck()) return;

				const bool pan = button == this->_controls.pan || this->_window->isKeyDown(this->_controls.panModifier);

				this->_looking = !pan;
				this->_panning = pan;
				this->_dragButton = button;
				this->_oldMousePos = mousePos;
			} else {
				if (button != this->_dragButton) return; // are we the dragging camera

				this->_looking = false;
				this->_panning = false;
				this->_dragButton = -1;
			}
		};

		this->_window->onMouseMove += [this](auto& /*w*/, const rawrbox::Vector2i& mousePos) {
			if (!this->_controlsEnabled || !this->isEnabled()) return;
			if (!this->_looking && !this->_panning) return;

			const auto deltaX = static_cast<float>(mousePos.x - this->_oldMousePos.x);
			const auto deltaY = static_cast<float>(mousePos.y - this->_oldMousePos.y);
			this->_oldMousePos = mousePos;

			if (this->_looking) {
				auto ang = this->getAngle();

				ang.x += this->_mouseSpeed * deltaX;
				ang.y = std::clamp(ang.y - this->_mouseSpeed * deltaY, -PITCH_LIMIT, PITCH_LIMIT);

				this->setAngle(ang);
			} else {
				// Panning
				const float scale = this->_panSpeed * this->_distance;
				this->setPos(this->getPos() + this->getRight() * (deltaX * scale) + this->getUp() * (deltaY * scale));
			}
		};
	}

	// CONTROLS ---
	void CameraOrbit::setControls(rawrbox::CameraOrbitControls controls) { this->_controls = controls; }
	const rawrbox::CameraOrbitControls& CameraOrbit::getControls() const { return this->_controls; }

	void CameraOrbit::enableControls(bool enabled) {
		this->_controlsEnabled = enabled;

		if (!enabled) {
			this->_looking = false;
			this->_panning = false;
			this->_dragButton = -1;
		}
	}

	void CameraOrbit::canUseMouse(std::function<bool()> check) { this->_mouseCheck = std::move(check); }
	void CameraOrbit::canUseKeyboard(std::function<bool()> check) { this->_keyboardCheck = std::move(check); }

	void CameraOrbit::setMoveSpeed(float speed) { this->_moveSpeed = speed; }
	void CameraOrbit::setMouseSpeed(float speed) { this->_mouseSpeed = speed; }
	// ------------

	// FOCUS ---
	rawrbox::Vector3f CameraOrbit::getPivot() const {
		return this->getPos() + this->getForward() * this->_distance;
	}

	void CameraOrbit::setPivot(const rawrbox::Vector3f& pivot) {
		this->setPos(pivot - this->getForward() * this->_distance);
	}

	void CameraOrbit::setDistance(float distance) {
		this->_distance = std::clamp(distance, this->_minDistance, this->_maxDistance);
	}

	void CameraOrbit::dolly(float steps) {
		this->setPos(this->getPos() + this->getForward() * (steps * this->_dollySpeed));
	}

	void CameraOrbit::orbitAround(const rawrbox::Vector3f& pos, std::optional<float> distance) {
		if (distance.has_value()) this->setDistance(distance.value());
		this->setPivot(pos);
	}
	// ---------

	void CameraOrbit::update() {
		if (this->_window == nullptr || !this->_controlsEnabled || !this->isEnabled()) return;
		if (this->_keyboardCheck != nullptr && !this->_keyboardCheck()) return; // For example, if we are typing on console or something else.

		const auto forward = this->getForward();
		const auto right = this->getRight();

		float speed = this->_moveSpeed;
		if (this->_window->isKeyDown(rawrbox::KEY_LEFT_SHIFT)) speed *= 2.F;
		speed *= rawrbox::DELTA_TIME;

		rawrbox::Vector3f move = {};

		if (this->_window->isKeyDown(this->_controls.forward)) move += forward;
		if (this->_window->isKeyDown(this->_controls.backwards)) move -= forward;

		if (this->_window->isKeyDown(this->_controls.left)) move += right;
		if (this->_window->isKeyDown(this->_controls.right)) move -= right;

		if (this->_window->isKeyDown(this->_controls.up)) move += rawrbox::Vector3f{0.F, 1.F, 0.F};
		if (this->_window->isKeyDown(this->_controls.down)) move -= rawrbox::Vector3f{0.F, 1.F, 0.F};

		const float len = move.length();
		if (len <= 0.0001F) return;

		this->setPos(this->getPos() + (move / len) * speed);
	}
} // namespace rawrbox
