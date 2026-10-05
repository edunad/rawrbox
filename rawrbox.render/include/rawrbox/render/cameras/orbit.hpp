#pragma once

#include <rawrbox/render/cameras/perspective.hpp>
#include <rawrbox/render/window.hpp>
#include <rawrbox/utils/keys.hpp>

#include <functional>
#include <optional>

namespace rawrbox {
	// Default Settings
	struct CameraOrbitControls {
		int look = rawrbox::MOUSE_BUTTON_2;
		int pan = rawrbox::MOUSE_BUTTON_3;
		int panModifier = rawrbox::KEY_LEFT_SHIFT;

		int forward = rawrbox::KEY_W;
		int backwards = rawrbox::KEY_S;
		int left = rawrbox::KEY_A;
		int right = rawrbox::KEY_D;
		
		int up = rawrbox::KEY_SPACE;
		int down = rawrbox::KEY_C;
	};
	// ---------

	class CameraOrbit : public rawrbox::CameraPerspective {
	protected:
		rawrbox::Window* _window = nullptr;

		// Camera control ---
		bool _controlsEnabled = true;

		bool _looking = false;
		bool _panning = false;

		int _dragButton = -1;

		float _moveSpeed = 8.F;
		float _mouseSpeed = 0.005F;
		float _panSpeed = 0.0015F;
		float _dollySpeed = 1.5F;

		float _minDistance = 0.25F;
		float _maxDistance = 200.F;

		rawrbox::CameraOrbitControls _controls = {};
		rawrbox::Vector2i _oldMousePos = {};

		std::function<bool()> _mouseCheck = nullptr;
		std::function<bool()> _keyboardCheck = nullptr;
		// ------------

		float _distance = 10.F;

	public:
		CameraOrbit(const CameraOrbit& other) = delete;
		CameraOrbit(CameraOrbit&& other) = default;
		CameraOrbit& operator=(const CameraOrbit&) = delete;
		CameraOrbit& operator=(CameraOrbit&&) = default;
		~CameraOrbit() override = default;

		explicit CameraOrbit(rawrbox::Window& window, float FOV = 60.F, float near = 0.01F, float far = 100.F);

		// CONTROLS ---
		void setControls(rawrbox::CameraOrbitControls controls);
		[[nodiscard]] const rawrbox::CameraOrbitControls& getControls() const;
		void enableControls(bool enabled);

		void canUseMouse(std::function<bool()> check);
		void canUseKeyboard(std::function<bool()> check);

		void setMoveSpeed(float speed);
		void setMouseSpeed(float speed);
		// ------------

		// FOCUS ---
		[[nodiscard]] rawrbox::Vector3f getPivot() const;
		void setPivot(const rawrbox::Vector3f& pivot);

		void setDistance(float distance);

		void dolly(float steps);
		void orbitAround(const rawrbox::Vector3f& pos, std::optional<float> distance = std::nullopt);
		// ---------

		void update() override;
	};
} // namespace rawrbox
