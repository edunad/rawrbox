#pragma once

#include <rawrbox/math/quaternion.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/camera/base.hpp>

namespace rawrbox {
	class CameraPerspective : public CameraBase {
	protected:
		void updateMtx() override;

	public:
		explicit CameraPerspective(float ratio, float FOV = 60.F, float near = 0.1F, float far = 100.F, bool homogeneousDepth = false);
	};
} // namespace rawrbox
