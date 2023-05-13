#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/camera/perspective.hpp>
#include <rawrbox/render/window.hpp>

#include <memory>

namespace rawrbox {
	class CameraOrbital : public CameraPerspective {
	protected:
		std::weak_ptr<rawrbox::Window> _window;

		// Camera control ---
		float _speed = 0.F;
		bool _rightClick = false;
		rawrbox::Vector2i _oldMousePos = {};
		// ------------
	public:
		explicit CameraOrbital(std::shared_ptr<rawrbox::Window> window, float speed = 8.F, float FOV = 60.F, float near = 0.1F, float far = 100.F, bool homogeneousDepth = false);
		void update() override;
	};
} // namespace rawrbox
