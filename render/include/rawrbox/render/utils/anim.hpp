#pragma once

#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>

#include <cmath>

namespace rawrbox {

	class AnimUtils {
	public:
		// ---
		static rawrbox::Vector4f lerpRotation(float time, std::pair<float, rawrbox::Vector4f> a, std::pair<float, rawrbox::Vector4f> b);
		static rawrbox::Vector3f lerpVector3(float time, std::pair<float, rawrbox::Vector3f> a, std::pair<float, rawrbox::Vector3f> b);
	};
} // namespace rawrbox
