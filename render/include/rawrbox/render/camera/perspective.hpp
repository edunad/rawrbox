#pragma once

#include <rawrbox/math/quaternion.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/camera/base.hpp>

namespace rawrBox {
	class CameraPerspective : public CameraBase {
	protected:
	public:
		CameraPerspective(float ratio, float FOV = 60.f, float near = 0.1f, float far = 100.f, bool homogeneousDepth = false);
		void update() override;
	};
} // namespace rawrBox
