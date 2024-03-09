#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/cameras/perspective.hpp>
#include <rawrbox/render/window.hpp>
#include <rawrbox/utils/keys.hpp>

#include <functional>

namespace rawrbox {

	struct CameraControls {
		int rotate = MOUSE_BUTTON_2;

		int forward = KEY_W;
		int backwards = KEY_S;
		int left = KEY_A;
		int right = KEY_D;
	};

	class CameraOrbital : public rawrbox::CameraPerspective {
	protected:
		rawrbox::Window* _window = nullptr;

		// Camera control ---
		bool _enabled = true;
		bool _look = false;

		float _moveSpeed = 8.F;
		float _mouseSpeed = 0.0015F;

		rawrbox::CameraControls _controls = {};
		rawrbox::Vector2i _oldMousePos = {};
		// ------------
	public:
		std::function<void()> onMovementStop = nullptr;
		std::function<void()> onMovementStart = nullptr;

		CameraOrbital(const CameraOrbital& other) = delete;
		CameraOrbital(CameraOrbital&& other) = default;
		CameraOrbital& operator=(const CameraOrbital&) = delete;
		CameraOrbital& operator=(CameraOrbital&&) = default;
		~CameraOrbital() override = default;

		explicit CameraOrbital(rawrbox::Window& window, float FOV = 60.F, float near = 0.01F, float far = 100.F);

		// UTILS ---
		void setControls(rawrbox::CameraControls controls);
		void enableControls(bool enabled);

		void setMoveSpeed(float speed);
		void setMouseSpeed(float speed);
		// ----

		void update() override;
	};
} // namespace rawrbox
