#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/camera/perspective.hpp>
#include <rawrbox/render/window.hpp>

#include <functional>

namespace rawrbox {
	class CameraOrbital : public rawrbox::CameraPerspective {
	protected:
		rawrbox::Window* _window = nullptr;

		// Camera control ---
		float _speed = 0.F;
		bool _rightClick = false;
		rawrbox::Vector2i _oldMousePos = {};
		// ------------
	public:
		std::function<void()> onMovementStop = nullptr;
		std::function<void()> onMovementStart = nullptr;

		CameraOrbital(const CameraOrbital& other) = default;
		CameraOrbital(CameraOrbital&& other) = default;
		CameraOrbital& operator=(const CameraOrbital&) = default;
		CameraOrbital& operator=(CameraOrbital&&) = default;
		~CameraOrbital() override = default;

		explicit CameraOrbital(rawrbox::Window& window, float speed = 8.F, float FOV = 60.F, float near = 0.1F, float far = 100.F);
		void update() override;
	};
} // namespace rawrbox
