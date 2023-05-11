#pragma once

#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>

#include <cmath>

namespace rawrbox {

	class AnimUtils {
	public:
		// taken from assimp, because linux /shrug
		static rawrbox::Vector4f interpolate(const rawrbox::Vector4f& pStart, const rawrbox::Vector4f& pEnd, float pFactor) {
			// calc cosine theta
			float cosom = pStart.x * pEnd.x + pStart.y * pEnd.y + pStart.z * pEnd.z + pStart.w * pEnd.w;

			// adjust signs (if necessary)
			rawrbox::Vector4f end = pEnd;
			if (cosom < static_cast<float>(0.0)) {
				cosom = -cosom;
				end.x = -end.x; // Reverse all signs
				end.y = -end.y;
				end.z = -end.z;
				end.w = -end.w;
			}

			// Calculate coefficients
			// NOLINTBEGIN(clang-analyzer-deadcode.DeadStores)
			float sclp = NAN;
			float sclq = NAN;

			if ((static_cast<float>(1.0) - cosom) > static_cast<float>(0.0001)) // 0.0001 -> some epsillon
			{
				// Standard case (slerp)
				float omega = NAN, sinom = NAN;
				omega = std::acos(cosom); // extract theta from dot product's cos theta
				sinom = std::sin(omega);
				sclp = std::sin((static_cast<float>(1.0) - pFactor) * omega) / sinom;
				sclq = std::sin(pFactor * omega) / sinom;
			} else {
				// Very close, do linear interp (because it's faster)
				sclp = static_cast<float>(1.0) - pFactor;
				sclq = pFactor;
			}
			// NOLINTEND(clang-analyzer-deadcode.DeadStores)

			rawrbox::Vector4f pOut;
			pOut.x = sclp * pStart.x + sclq * end.x;
			pOut.y = sclp * pStart.y + sclq * end.y;
			pOut.z = sclp * pStart.z + sclq * end.z;
			pOut.w = sclp * pStart.w + sclq * end.w;

			return pOut;
		};

		static void normalize(rawrbox::Vector4f& quart) {
			// compute the magnitude and divide through it
			const float mag = std::sqrt(quart.x * quart.x + quart.y * quart.y + quart.z * quart.z + quart.w * quart.w);

			if (mag) {
				const float invMag = static_cast<float>(1.0) / mag;

				quart.x *= invMag;
				quart.y *= invMag;
				quart.z *= invMag;
				quart.w *= invMag;
			}
		}

		// ---
		static rawrbox::Vector4f lerpRotation(float time, std::pair<float, rawrbox::Vector4f> a, std::pair<float, rawrbox::Vector4f> b) {
			if (a.first == b.first) return a.second;

			float dt = b.first - a.first;
			float norm = (time - a.first) / dt;

			rawrbox::Vector4f aiStart = {a.second.x, a.second.y, a.second.z, a.second.w};
			rawrbox::Vector4f aiEnd = {b.second.x, b.second.y, b.second.z, b.second.w};
			rawrbox::Vector4f aiIntrp = AnimUtils::interpolate(aiStart, aiEnd, norm);

			AnimUtils::normalize(aiIntrp);
			return {aiIntrp.x, aiIntrp.y, aiIntrp.z, aiIntrp.w};
		};

		static rawrbox::Vector3f lerpVector3(float time, std::pair<float, rawrbox::Vector3f> a, std::pair<float, rawrbox::Vector3f> b) {
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
	};
} // namespace rawrbox
