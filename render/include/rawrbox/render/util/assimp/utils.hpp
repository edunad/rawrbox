#pragma once

#include <rawrbox/math/quaternion.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/model/model.h>

#include <assimp/quaternion.h>

#include <cmath>

namespace rawrBox {
	class AssimpUtils {
	public:
		// taken from assimp, because linux /shrug
		static aiQuaternion interpolate(const aiQuaternion& pStart, const aiQuaternion& pEnd, float pFactor) {
			// calc cosine theta
			float cosom = pStart.x * pEnd.x + pStart.y * pEnd.y + pStart.z * pEnd.z + pStart.w * pEnd.w;

			// adjust signs (if necessary)
			aiQuaternion end = pEnd;
			if (cosom < static_cast<float>(0.0)) {
				cosom = -cosom;
				end.x = -end.x; // Reverse all signs
				end.y = -end.y;
				end.z = -end.z;
				end.w = -end.w;
			}

			// Calculate coefficients
			float sclp = NAN, sclq = NAN;
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

			aiQuaternion pOut;
			pOut.x = sclp * pStart.x + sclq * end.x;
			pOut.y = sclp * pStart.y + sclq * end.y;
			pOut.z = sclp * pStart.z + sclq * end.z;
			pOut.w = sclp * pStart.w + sclq * end.w;

			return pOut;
		};

		static void normalize(aiQuaternion& quart) {
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
		static aiQuaternion lerpRotation(float time, rawrBox::AnimKey<aiQuaternion> a, rawrBox::AnimKey<aiQuaternion> b) {
			if (a.time == b.time) return a.value;

			float dt = b.time - a.time;
			float norm = (time - a.time) / dt;

			aiQuaternion aiStart = {a.value.w, a.value.x, a.value.y, a.value.z};
			aiQuaternion aiEnd = {b.value.w, b.value.x, b.value.y, b.value.z};
			aiQuaternion aiIntrp = AssimpUtils::interpolate(aiStart, aiEnd, norm);

			AssimpUtils::normalize(aiIntrp);
			return {aiIntrp.x, aiIntrp.y, aiIntrp.z, aiIntrp.w};
		};

		static aiVector3D lerpPosition(float time, rawrBox::AnimKey<aiVector3D> a, rawrBox::AnimKey<aiVector3D> b) {
			if (a.time == b.time) return a.value;

			float dt = b.time - a.time;
			float norm = (time - a.time) / dt;

			a.value.x *= (1.0f - norm);
			a.value.y *= (1.0f - norm);
			a.value.z *= (1.0f - norm);

			b.value.x *= norm;
			b.value.y *= norm;
			b.value.z *= norm;

			return aiVector3D(
			    a.value.x + b.value.x,
			    a.value.y + b.value.y,
			    a.value.z + b.value.z);
		};

		static aiVector3D lerpScale(float time, rawrBox::AnimKey<aiVector3D> a, rawrBox::AnimKey<aiVector3D> b) {
			if (a.time == b.time) return a.value;

			float dt = b.time - a.time;
			float norm = (time - a.time) / dt;

			return lerpPosition(norm, a, b);
		};
	};
} // namespace rawrBox
