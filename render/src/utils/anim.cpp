
#include <rawrbox/render/utils/anim.hpp>

namespace rawrbox {
	rawrbox::Vector4f AnimUtils::lerpRotation(float time, std::pair<float, rawrbox::Vector4f> a, std::pair<float, rawrbox::Vector4f> b) {
		if (a.first == b.first) return a.second;

		float dt = b.first - a.first;
		float norm = (time - a.first) / dt;

		rawrbox::Vector4f aiStart = {a.second.x, a.second.y, a.second.z, a.second.w};
		rawrbox::Vector4f aiEnd = {b.second.x, b.second.y, b.second.z, b.second.w};
		rawrbox::Vector4f aiIntrp = aiStart.interpolate(aiEnd, norm);

		auto out = aiIntrp.normalized();
		return {out.x, out.y, out.z, out.w};
	};

	rawrbox::Vector3f AnimUtils::lerpVector3(float time, std::pair<float, rawrbox::Vector3f> a, std::pair<float, rawrbox::Vector3f> b) {
		if (a.first == b.first) return a.second;

		float dt = b.first - a.first;
		float norm = (time - a.first) / dt;

		a.second.x *= (1.0F - norm);
		a.second.y *= (1.0F - norm);
		a.second.z *= (1.0F - norm);

		b.second.x *= norm;
		b.second.y *= norm;
		b.second.z *= norm;

		return {
		    a.second.x + b.second.x,
		    a.second.y + b.second.y,
		    a.second.z + b.second.z};
	};
} // namespace rawrbox
